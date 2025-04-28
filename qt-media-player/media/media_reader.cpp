#include "media_reader.h"
#include <QDebug>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#ifdef DEBUG_VIDEO_FRAME_WRITE

#endif // DEBUG_VIDEO_FRAME_WRITE


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
    // 打开流
    if (avformat_open_input(&m_formatContext, pParam.sUri.c_str(), nullptr, nullptr) != 0)
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

    if(m_videoStreamIndex < 0 && m_audioStreamIndex < 0)
    {
        qDebug() << "find video stream and audio stream failed";
        avformat_close_input(&m_formatContext);
        avformat_network_deinit();
        return false;
    }

    if(m_videoStreamIndex >=0)
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
        if (avcodec_open2(m_videoCodecContext, videoCodec, nullptr) != 0)
        {
            qDebug() << "avcodec_open2 failed" ;
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }

        if (m_videoCodecContext->width <= 0 || m_videoCodecContext->height <= 0 || m_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
        {
            qDebug() << "codecContext data error" ;
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
    }
    if(m_audioStreamIndex >=0)
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
    if (m_audioSwsContext)
    {
        sws_freeContext(m_audioSwsContext);
        m_audioSwsContext = nullptr;
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

void MediaReader::FrameReader()
{
    m_bFrameReaderRunning = true;
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    const int      in_sample_rate = m_audioCodecContext->sample_rate;
    AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
    uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
    int            in_channels = m_audioCodecContext->channels;
    const int      out_sample_rate = 16000;
    AVSampleFormat out_sfmt = AV_SAMPLE_FMT_S16;
    uint64_t       out_channel_layout = AV_CH_LAYOUT_MONO;
    int            out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    int            out_spb = av_get_bytes_per_sample(out_sfmt);


    AVPacket* packet = av_packet_alloc();
    bool isStreamOver = false;
    while (m_bFrameReaderRunning.load() && !isStreamOver)
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
            {
                isStreamOver = true;
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
}

void MediaReader::VideoDecode(AVPacket* packet)
{
    if (packet->stream_index != m_videoStreamIndex)
        return;
    //bool isKeyPacket = packet->flags & AV_PKT_FLAG_KEY;//关键帧
    int ret = avcodec_send_packet(m_videoCodecContext, packet);
    if (ret < 0)
    {//错误处理
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_videoCodecContext, frame);
        if (ret < 0)
        {//错误处理
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        if (frame->key_frame == 1)
        {//关键帧
        }
        
        std::lock_guard<std::mutex> lock(m_videoFrameQueueMutex);
        m_videoFrameQueue.push(frame);
        m_videoFrameQueueCV.notify_all();
        //av_frame_unref(frame);//不可解引用
    }
}

void MediaReader::AudioDecode(AVPacket* packet)
{
    if (packet->stream_index != m_audioStreamIndex)
        return;

    bool ret = avcodec_send_packet(m_audioCodecContext, packet);
    if (ret < 0)
    {//错误处理
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_audioCodecContext, frame);
        if (ret < 0)
        {//错误处理
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        std::lock_guard<std::mutex> lock(m_audioFrameQueueMutex);
        m_audioFrameQueue.push(frame);
        av_frame_unref(frame);
    }
}

void MediaReader::VideoProcess()
{
    m_bVideoProcessRunning.store(true);
    if(!m_videoSwsContext)
    {
        m_videoSwsContext = sws_getCachedContext(NULL,
                                                 m_videoCodecContext->width, m_videoCodecContext->height, m_videoCodecContext->pix_fmt,
                                                 m_videoCodecContext->width, m_videoCodecContext->height, AV_PIX_FMT_BGR24,
                                                 SWS_BILINEAR, NULL, NULL, NULL);
    }
    // 创建RGB视频帧
    AVFrame* frameRGB = av_frame_alloc();
    int      numBytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t)); //注意，这里给frameRGB申请的buffer，需要单独释放
    av_image_fill_arrays(
                frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_BGR24, m_videoCodecContext->width, m_videoCodecContext->height, AV_INPUT_BUFFER_PADDING_SIZE
                );

    int frameIndex = 0;
    while (m_bVideoProcessRunning.load())
    {
        std::unique_lock<std::mutex> lock(m_videoFrameQueueMutex);
        m_videoFrameQueueCV.wait(lock,[this](){
            return !m_videoFrameQueue.empty() || !m_bVideoProcessRunning.load();
        });
        if(!m_bVideoProcessRunning.load())
            break;

        AVFrame* frame = m_videoFrameQueue.front();
        m_videoFrameQueue.pop();
        lock.unlock();//数据取出，提前解锁

        frameIndex++;
        int ret = sws_scale(m_videoSwsContext, frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
        if(m_param.videoCallback)
        {
            cv::Mat mat = cv::Mat(frame->height, frame->width, CV_8UC3, frameRGB->data[0], frameRGB->linesize[0]);
#ifdef DEBUG_VIDEO_FRAME_WRITE
            char filename[100];
            sprintf(filename, "%d.jpg", frameIndex);
            cv::imwrite(filename, mat);
#endif // DEBUG_VIDEO_FRAME_WRITE

            VideoFrame vframe;
            vframe.videoData = mat.clone();
            m_param.videoCallback(vframe);
        }
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
    av_frame_free(&frameRGB);
    av_freep(&buffer);
}

void MediaReader::AudioProcess()
{

}
