#include "media_reader.h"
#include <QDebug>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#ifdef DEBUG_VIDEO_FRAME_WRITE

#endif // DEBUG_VIDEO_FRAME_WRITE

#ifdef DEBUG_AUDIO_RESAMPLE_WRITE

#endif // DEBUG_AUDIO_RESAMPLE_WRITE

MediaReader::MediaReader()
{

}

MediaReader::~MediaReader()
{
    if (m_thFrameReader.joinable())
        m_thFrameReader.join();
}

bool MediaReader::Init(const MediaParameter& pParam)
{
    m_param = pParam;

    avformat_network_init();

    //配置该流的ffmpeg设置
    AVDictionary* pOptDict = NULL;
    av_dict_set(&pOptDict, "stimeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间，可能不生效
    av_dict_set(&pOptDict, "timeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间
    av_dict_set(&pOptDict, "buffer_size", "8192000", 0);//控制解码器或编码器的内部缓冲区大小,配置8M缓冲以适应高分辨率视频
    av_dict_set(&pOptDict, "recv_buffer_size", "4096000", 0);     // 防止花屏, max 4M.:用于控制网络接收缓冲区大小，适用于高带宽或高延迟的网络环境
    av_dict_set(&pOptDict, "tune", "stillimage,fastdecode,zerolatency", 0);//优化静态图像编码,快速解码和低延时传输
    av_dict_set(&pOptDict, "rtsp_transport", "tcp", 0);//tcp拉流，尽量保证不丢包
    int ret = avformat_open_input(&m_formatContext, pParam.sUri.c_str(), nullptr, &pOptDict);
    av_dict_free(&pOptDict);
    pOptDict = nullptr;

    // 打开流
    if (ret != 0)
    {
        qDebug() << "avformat_open_input failed";
        avformat_network_deinit();
        return false;
    }

    // 获取流信息
    if (avformat_find_stream_info(m_formatContext, nullptr) < 0)
    {
        qDebug() << "avformat_find_stream_info failed";
        avformat_close_input(&m_formatContext);
        avformat_network_deinit();
        return false;
    }

    // 寻找视频流和音频流，这里只取一次
    for (unsigned int i = 0; i < m_formatContext->nb_streams; ++i)
    {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIndex < 0)
        {
            m_videoStreamIndex = i;
        }
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIndex < 0)
        {
            m_audioStreamIndex = i;
        }
    }

    if (m_videoStreamIndex < 0 && m_audioStreamIndex < 0)
    {
        qDebug() << "find video stream and audio stream failed";
        avformat_close_input(&m_formatContext);
        avformat_network_deinit();
        return false;
    }

    if (m_videoStreamIndex >= 0)
    {
        // 获取视频流解码器参数
        AVCodecParameters* videoCodecParameters = m_formatContext->streams[m_videoStreamIndex]->codecpar;
        // 获取视频解码器
        const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
        // 创建解码器上下文
        m_videoCodecContext = avcodec_alloc_context3(videoCodec);
        if (avcodec_parameters_to_context(m_videoCodecContext, videoCodecParameters) < 0)
        {
            qDebug() << "avcodec_parameters_to_context failed";
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
        m_videoCodecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
        if (avcodec_open2(m_videoCodecContext, videoCodec, nullptr) != 0)
        {
            qDebug() << "avcodec_open2 failed";
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }

        if (m_videoCodecContext->width <= 0 || m_videoCodecContext->height <= 0 || m_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
        {
            qDebug() << "codecContext data error";
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
    }
    if (m_audioStreamIndex >= 0)
    {
        // 获取音频流解码器参数
        AVCodecParameters* audiooCodecParameters = m_formatContext->streams[m_audioStreamIndex]->codecpar;
        // 获取视频解码器
        const AVCodec* audioCodec = avcodec_find_decoder(audiooCodecParameters->codec_id);
        // 创建解码器上下文
        m_audioCodecContext = avcodec_alloc_context3(audioCodec);
        if (avcodec_parameters_to_context(m_audioCodecContext, audiooCodecParameters) < 0)
        {
            qDebug() << "avcodec_parameters_to_context failed";
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
        if (avcodec_open2(m_audioCodecContext, audioCodec, nullptr) != 0)
        {
            qDebug() << "avcodec_open2 failed";
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
    }
    return true;
}

void MediaReader::UnInit()
{
    if (m_videoSwsContext)
    {
        sws_freeContext(m_videoSwsContext);
        m_videoSwsContext = nullptr;
    }
    if (m_audioSwrContext)
    {
        swr_free(&m_audioSwrContext);
        m_audioSwrContext = nullptr;
    }
    if (m_videoCodecContext)
    {
        if (avcodec_is_open(m_videoCodecContext))
            avcodec_close(m_videoCodecContext);
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    if (m_audioCodecContext)
    {
        if (avcodec_is_open(m_audioCodecContext))
            avcodec_close(m_audioCodecContext);
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }
    if (m_formatContext)
    {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }

    avformat_network_deinit();
}

bool MediaReader::Start()
{
    if (!m_thFrameReader.joinable())
    {
        m_thFrameReader = std::thread(&MediaReader::FrameReader, this);
    }
    if (!m_thVideoProcess.joinable())
    {
        m_thVideoProcess = std::thread(&MediaReader::VideoProcess, this);
    }
    if (!m_thAudioProcess.joinable())
    {
        m_thAudioProcess = std::thread(&MediaReader::AudioProcess, this);
    }
    return true;
}

bool MediaReader::Stop()
{
    return false;
}

int g_videoFrameIndex = 0;
int g_audioFrameIndex = 0;
void MediaReader::FrameReader()
{
    m_bFrameReaderRunning = true;
    AVPacket* packet = av_packet_alloc();
    while (m_bFrameReaderRunning.load() && !m_bStreamOver.load())
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            qDebug() << "av_read_frame err:" << ret;
            if (ret == AVERROR_EOF)
            {
                m_bStreamOver = true;
                qDebug() << "FrameReader stream over";
            }
            else if (ret == AVERROR(EAGAIN))
            {//重试
            }
            else if (ret == AVERROR(ENOMEM))
            {//内存不足
                break;
            }
            continue;
        }
        if (packet->stream_index == m_videoStreamIndex)
        {
            VideoDecode(packet);
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {
            AudioDecode(packet);
        }

    }
    //av_packet_unref(packet);
    //av_packet_free(&packet);
    //av_frame_free(&frame);
    //av_frame_free(&frameRGB);
    //av_freep(&buffer);
    qDebug() << "FrameReader end";
}

void MediaReader::VideoDecode(AVPacket* packet)
{
    if (packet->stream_index != m_videoStreamIndex)
        return;

    {
        std::unique_lock<std::mutex> lock(m_videoFrameQueueMutex);
        m_videoFrameQueueNotFullCV.wait(lock, [this]() {
            return m_videoFrameQueue.size() < kMaxVideoFrameQueueSize || !m_bVideoProcessRunning.load();
            });
        lock.unlock();
        if (!m_bVideoProcessRunning.load())
            return;
    }
    //bool isKeyPacket = packet->flags & AV_PKT_FLAG_KEY;//关键帧
    int ret = avcodec_send_packet(m_videoCodecContext, packet);
    if (ret < 0)
    {//错误处理
        qDebug() << "avcodec_send_packet err:" << ret;
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_videoCodecContext, frame);
        if (ret < 0)
        {//错误处理
            qDebug() << "avcodec_receive_frame err:" << ret;
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        if (frame->key_frame == 1)
        {//关键帧
        }
        qDebug() << "decode video frame index:" << g_videoFrameIndex;
        g_videoFrameIndex++;
        //av_frame_unref(frame);
        //av_frame_free(&frame);
        std::lock_guard<std::mutex> lock(m_videoFrameQueueMutex);
        m_videoFrameQueue.push(frame);
        m_videoFrameQueueNotEmptyCV.notify_all();
        //av_frame_unref(frame);//不可解引用
    }
}

void MediaReader::AudioDecode(AVPacket* packet)
{
    if (packet->stream_index != m_audioStreamIndex)
        return;

    {
        std::unique_lock<std::mutex> lock(m_audioFrameQueueMutex);
        m_audioFrameQueueNotFullCV.wait(lock, [this]() {
            return m_audioFrameQueue.size() < kMaxAudioFrameQueueSize || !m_bAudioProcessRunning.load();
            });
        lock.unlock();
        if (!m_bAudioProcessRunning.load())
            return;
    }

    int ret = avcodec_send_packet(m_audioCodecContext, packet);
    if (ret < 0)
    {//错误处理
        qDebug() << "avcodec_send_packet err:" << ret;
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_audioCodecContext, frame);
        if (ret < 0)
        {//错误处理
            qDebug() << "avcodec_receive_frame err:" << ret;
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        qDebug() << "decode audio frame index:" << g_audioFrameIndex;
        g_audioFrameIndex++;
        std::lock_guard<std::mutex> lock(m_audioFrameQueueMutex);
        m_audioFrameQueue.push(frame);
        m_audioFrameQueueNotEmptyCV.notify_all();
        //av_frame_unref(frame);
    }
}

void MediaReader::VideoProcess()
{
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;
    AVPixelFormat dstFormat = AV_PIX_FMT_RGB24;

    m_bVideoProcessRunning.store(true);
    if (!m_videoSwsContext)
    {
        m_videoSwsContext = sws_getCachedContext(NULL,
            m_videoCodecContext->width, m_videoCodecContext->height, m_videoCodecContext->pix_fmt,
            m_videoCodecContext->width, m_videoCodecContext->height, dstFormat,
            SWS_BILINEAR, NULL, NULL, NULL);
    }
    // 创建RGB视频帧
    AVFrame* frameRGB = av_frame_alloc();
    int      numBytes = av_image_get_buffer_size(dstFormat, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t)); //注意，这里给frameRGB申请的buffer，需要单独释放
    av_image_fill_arrays(
        frameRGB->data, frameRGB->linesize, buffer, dstFormat, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE
    );

    int frameIndex = 0;
    while (m_bVideoProcessRunning.load())
    {
        std::unique_lock<std::mutex> lock(m_videoFrameQueueMutex);
        m_videoFrameQueueNotEmptyCV.wait(lock, [this]() {
            return !m_videoFrameQueue.empty() || !m_bVideoProcessRunning.load();
            });
        if (!m_bVideoProcessRunning.load())
            break;

        AVFrame* frame = m_videoFrameQueue.front();
        m_videoFrameQueue.pop();
        lock.unlock();//数据取出，提前解锁

        frameIndex++;
        int ret = sws_scale(m_videoSwsContext, frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
        if (m_param.videoCallback)
        {
            cv::Mat mat = cv::Mat(frame->height, frame->width, CV_8UC3, frameRGB->data[0], frameRGB->linesize[0]);

            bool isTooGray = false;
            {
                // 将帧转换为灰度图像
                cv::Mat grayFrame;
                //检查灰度可以压缩图像大小，以加速检查
                cv::resize(mat, grayFrame, cv::Size(100, 100));
                cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);
                // 计算灰度图像的方差
                cv::Scalar mean, stddev;
                cv::meanStdDev(grayFrame, mean, stddev);
                double dGrayScaleDegree = stddev[0] * stddev[0];
                if (dGrayScaleDegree < 100.0) {
                    isTooGray = true;
                    // cv::imshow("gray",matRgb);
                    // cv::waitKey(1);
                }
            }
            if (isTooGray)
                continue;
#ifdef DEBUG_VIDEO_FRAME_WRITE
            //opencv默认使用的是BGR格式，需要进行转换
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
            char filename[100];
            sprintf(filename, "%d.jpg", frameIndex);
            cv::imwrite(filename, mat);
#endif // DEBUG_VIDEO_FRAME_WRITE
            VideoFrame vframe;
            vframe.videoData = mat.clone();
            double frameDelay = (frame->pts - m_lastVideoFramePts) * av_q2d(videoStream->time_base) * 1000000;
            qDebug() << "frameDelay:" << frameDelay;
            av_usleep(frameDelay);
            m_param.videoCallback(vframe);
            m_lastVideoFramePts = frame->pts;
            m_videoFrameQueueNotFullCV.notify_all();
        }
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
    av_frame_free(&frameRGB);
    av_freep(&buffer);
}

void MediaReader::AudioProcess()
{
    const int      in_sample_rate = m_audioCodecContext->sample_rate;
    AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
    int            int_spb = av_get_bytes_per_sample(in_sfmt);
    uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
    int            in_channels = m_audioCodecContext->channels;

    const int      out_sample_rate = m_param.outputSampleRate;// 16000;
    AVSampleFormat out_sfmt = AV_SAMPLE_FMT_NONE;
    if (m_param.outputBitPerSample == 8)
        out_sfmt = AV_SAMPLE_FMT_U8;
    else if (m_param.outputBitPerSample == 16)
        out_sfmt = AV_SAMPLE_FMT_S16;
    else if (m_param.outputBitPerSample == 32)
        out_sfmt = AV_SAMPLE_FMT_S32;

    int            out_spb = av_get_bytes_per_sample(out_sfmt);
    uint64_t       out_channel_layout = AV_CH_LAYOUT_MONO;
    if (m_param.outputChannelCount == 1)
        out_channel_layout = AV_CH_LAYOUT_MONO;
    else if (m_param.outputChannelCount == 2)
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    int            out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    m_bAudioProcessRunning.store(true);
    bool bNeedResample = false;
    if ((in_sample_rate != out_sample_rate) || (in_sfmt != out_sfmt) || (in_channels != out_channels))
        bNeedResample = true;

    // 创建重采样上下文
    if (bNeedResample)
    {
        m_audioSwrContext = swr_alloc_set_opts(nullptr, out_channel_layout, out_sfmt, out_sample_rate, in_channel_layout, in_sfmt, in_sample_rate, 0, NULL);
        if (!m_audioSwrContext)
        {
            // 处理重采样上下文创建失败的情况
            qDebug() << "swr_alloc_set_opts failed";
            return;
        }
        if (swr_init(m_audioSwrContext) != 0)
        {
            qDebug() << "swr_alloc_set_opts failed";
            return;
        }
    }

    int frameIndex = 0;
    while (m_bAudioProcessRunning.load())
    {
        std::unique_lock<std::mutex> lock(m_audioFrameQueueMutex);
        m_audioFrameQueueNotEmptyCV.wait(lock, [this]() {
            return !m_audioFrameQueue.empty() || !m_bAudioProcessRunning.load();
            });
        if (!m_bAudioProcessRunning.load())
            break;

        AVFrame* frame = m_audioFrameQueue.front();
        m_audioFrameQueue.pop();
        lock.unlock();//数据取出，提前解锁

        frameIndex++;
        if (bNeedResample && m_audioSwrContext)
        {
            // 计算重采样输出采样点数
            int max_out_nb_samples = av_rescale_rnd(
                swr_get_delay(m_audioSwrContext, frame->sample_rate) + frame->nb_samples, out_sample_rate, frame->sample_rate, AV_ROUND_UP
            );
            AVFrame* frameResample = av_frame_alloc();
            //使用av_samples_alloc时,结束后需要调用av_freep(&audio_data[0])释放内存,否则会内存泄漏
            av_samples_alloc(frameResample->data, frameResample->linesize, out_channels, max_out_nb_samples, out_sfmt, 1);

            int out_nb_samples = swr_convert(m_audioSwrContext, frameResample->data, max_out_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
            qDebug() << "succeed to convert frame " << frameIndex++ << " samples[" << frame->nb_samples << "]->[" << out_nb_samples << "]";

            if (m_param.audioCallback)
            {
                AudioFrame aframe;
                aframe.audioData = frameResample->data[0];
                aframe.dataSize = out_spb * out_channels * out_nb_samples;
                aframe.sampleRate = m_param.outputSampleRate;
                aframe.bitPerSample = m_param.outputBitPerSample;
                aframe.channelCount = m_param.outputChannelCount;
                m_param.audioCallback(aframe);
                m_audioFrameQueueNotFullCV.notify_all();
            }
            av_freep(&frameResample->data[0]);
            av_frame_unref(frameResample);
            av_frame_free(&frameResample);
        }
        else
        {
            if (m_param.audioCallback)
            {
                AudioFrame aframe;
                aframe.audioData = frame->data[0];
                aframe.dataSize = frame->linesize[0];
                aframe.sampleRate = m_param.outputSampleRate;
                aframe.bitPerSample = m_param.outputBitPerSample;
                aframe.channelCount = m_param.outputChannelCount;
                m_param.audioCallback(aframe);
                m_audioFrameQueueNotFullCV.notify_all();
            }
        }
        av_frame_unref(frame);
        av_frame_free(&frame);
    }

    if (bNeedResample && m_audioSwrContext)
    {
        int      max_cache_out_nb_samples = 2048;
        AVFrame* frameResample = av_frame_alloc();
        //使用av_samples_alloc时,结束后需要调用av_freep(&audio_data[0])释放内存,否则会内存泄漏
        av_samples_alloc(frameResample->data, frameResample->linesize, out_channels, max_cache_out_nb_samples, out_sfmt, 1);

        int out_cache_nb_samples = swr_convert(m_audioSwrContext, frameResample->data, max_cache_out_nb_samples, nullptr, 0);
        qDebug() << "get cache convert samples " << out_cache_nb_samples;

        if (m_param.audioCallback)
        {
            AudioFrame aframe;
            aframe.audioData = frameResample->data[0];
            aframe.dataSize = out_spb * out_channels * out_cache_nb_samples;
            aframe.sampleRate = m_param.outputSampleRate;
            aframe.bitPerSample = m_param.outputBitPerSample;
            aframe.channelCount = m_param.outputChannelCount;
            m_param.audioCallback(aframe);
            m_audioFrameQueueNotFullCV.notify_all();
        }
        av_freep(&frameResample->data[0]);
        av_frame_unref(frameResample);
        av_frame_free(&frameResample);
    }
}
