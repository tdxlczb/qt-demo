#include "media_reader.h"
#include <QDebug>
#include "utils/jitter_buffer.h"
#include "media/media_display.h"

#ifdef DEBUG_VIDEO_FRAME_WRITE

#endif // DEBUG_VIDEO_FRAME_WRITE

#ifdef DEBUG_AUDIO_RESAMPLE_WRITE

#endif // DEBUG_AUDIO_RESAMPLE_WRITE

//获取最大公约数
static int GetGCD(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

FrameQueue::FrameQueue(int16_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize)
{
}

FrameQueue::~FrameQueue()
{
}

void FrameQueue::Push(AVFrame* frame)
{
    std::unique_lock<std::mutex> lock(m_frameQueueMutex);
    // 当队列满了，阻塞在这里
    // 停止播放时，通过Clear清空队列可以解除阻塞
    m_queueCV.wait(lock, [this]() {
        return m_frameQueue.size() < m_maxQueueSize;
        });
    //lock.unlock();
    m_frameQueue.push(frame);
}

AVFrame* FrameQueue::PopFront()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    if (m_frameQueue.empty())
        return nullptr;
    AVFrame* frame = m_frameQueue.front();
    m_frameQueue.pop();
    m_queueCV.notify_all();
    return frame;
}

void FrameQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    std::queue<AVFrame*> empty;
    std::swap(empty, m_frameQueue);
}


std::string av_error_string(int errnum) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, buf, sizeof(buf));
    return std::string(buf);
}

QString av_error_qstring(int errnum) {
    char buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, buf, sizeof(buf));
    return QString(buf);
}

static AVPixelFormat GetHwFormat(AVCodecContext* pCodecContext, const enum AVPixelFormat* pPixFmts)
{
    const enum AVPixelFormat* pPixFmt;
    MediaReader* pReader = (MediaReader*)pCodecContext->opaque;

    //遍历给定的AVPixelFormat数组中存不存在AVCodecContext支持的硬解码格式
    for (pPixFmt = pPixFmts; *pPixFmt != AV_PIX_FMT_NONE; ++pPixFmt)
    {
        if (*pPixFmt == pReader->GetHwPixFmt())
        {
            return *pPixFmt;
        }
    }
    qWarning() << "HwPixFmt not found " << pReader->GetHwPixFmt();
    pReader->QuitHwDecode();
    return AV_PIX_FMT_NONE;
}

MediaReader::MediaReader()
{

}

MediaReader::~MediaReader()
{
    if (m_thFrameReader.joinable())
        m_thFrameReader.join();
}

const AVPixelFormat& MediaReader::GetHwPixFmt() const
{
    return m_hwPixFmt;
}

void MediaReader::QuitHwDecode()
{
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
}

bool MediaReader::Init(const MediaParameter& param)
{
    m_param = param;

    avformat_network_init();

    qDebug() << "open url:" << QString::fromStdString(param.url);
    //配置该流的ffmpeg设置
    AVDictionary* pOptDict = NULL;
    av_dict_set(&pOptDict, "stimeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间，可能不生效
    av_dict_set(&pOptDict, "timeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间
    av_dict_set(&pOptDict, "buffer_size", "8192000", 0);//控制解码器或编码器的内部缓冲区大小,配置8M缓冲以适应高分辨率视频
    av_dict_set(&pOptDict, "recv_buffer_size", "4096000", 0);     // 防止花屏, max 4M.:用于控制网络接收缓冲区大小，适用于高带宽或高延迟的网络环境
    av_dict_set(&pOptDict, "tune", "stillimage,fastdecode,zerolatency", 0);//优化静态图像编码,快速解码和低延时传输
    av_dict_set(&pOptDict, "rtsp_transport", "tcp", 0);//tcp拉流，尽量保证不丢包
    int ret = avformat_open_input(&m_formatContext, param.url.c_str(), nullptr, &pOptDict);
    av_dict_free(&pOptDict);
    pOptDict = nullptr;

    // 打开流
    if (AVERROR(ret))
    {
        qDebug() << "avformat_open_input failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
        avformat_network_deinit();
        return false;
    }

    // 获取流信息
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0)
    {
        qDebug() << "avformat_find_stream_info failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
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
            qDebug() << "avcodec_parameters_to_context failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
        //m_videoCodecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
        m_videoCodecContext->opaque = this; //用于回调里获取的指针
        m_videoCodecContext->thread_count = 3;

        if (!param.hwDeviceName.empty()) {
            m_hwDeviceType = av_hwdevice_find_type_by_name(param.hwDeviceName.c_str());
            if (m_hwDeviceType == AV_HWDEVICE_TYPE_NONE) {
                qDebug() << "device type is not supported:" << QString::fromStdString(param.hwDeviceName);
                return false;
            }
            //查找硬解码器
            for (int i = 0;; ++i) {
                const AVCodecHWConfig* codecHWConfig = avcodec_get_hw_config(videoCodec, i);
                if (!codecHWConfig) {
                    //没有硬解码器了
                    qWarning() << QString("decoder %1 is not support device type:%2").arg(videoCodec->name).arg(QString(av_hwdevice_get_type_name(m_hwDeviceType)));
                    break;
                }
                if (codecHWConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                    codecHWConfig->device_type == m_hwDeviceType) {
                    //找到了指定的解码器，记录对应的AVPixelFormat，把硬件支持的像素格式设置进去，后面get_format需要使用;
                    m_hwPixFmt = codecHWConfig->pix_fmt;
                    break;
                }
            }
            //如果上下文已经有了硬解码，那么将其取消引用，准备重新创建
            if (m_videoCodecContext->hw_device_ctx) {
                av_buffer_unref(&m_videoCodecContext->hw_device_ctx);
                m_videoCodecContext->hw_device_ctx = nullptr;
            }
            // 创建硬解码器
            ret = av_hwdevice_ctx_create(&m_videoCodecContext->hw_device_ctx, m_hwDeviceType, nullptr, nullptr, 0);
            if (AVERROR(ret)) {
                qDebug() << "av_hwdevice_ctx_create failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
            }
            m_videoCodecContext->get_format = GetHwFormat;
        }

        ret = avcodec_open2(m_videoCodecContext, videoCodec, nullptr);
        if (ret != 0) {
            qDebug() << "avcodec_open2 failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
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
        ret = avcodec_parameters_to_context(m_audioCodecContext, audiooCodecParameters);
        if (AVERROR(ret)) {
            qDebug() << "avcodec_parameters_to_context failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
        ret = avcodec_open2(m_audioCodecContext, audioCodec, nullptr);
        if (ret != 0) {
            qDebug() << "avcodec_open2 failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            avformat_network_deinit();
            return false;
        }
    }

    if (!m_pVideoDisplay) {
        VideoSpec spec = param.outputVideoSpec;
        m_pVideoDisplay = new VideoDisplay(1, spec);
        m_pVideoDisplay->SetCallback(std::bind(&MediaReader::DisplayVideo, this, std::placeholders::_1));
    }
    if (!m_pAudioDisplay) {
        AudioSpec spec = param.outputAudioSpec;
        m_pAudioDisplay = new AudioDisplay(1, spec);
    }
    //if (!m_audioBuffer->IsInit())
    //{
    //    m_audioBuffer->Init<int8_t>();
    //}
    return true;
}

void MediaReader::UnInit()
{
    if (m_pVideoDisplay)
    {
        delete m_pVideoDisplay;
        m_pVideoDisplay = nullptr;
    }
    if (m_pGrayFrameDisplay)
    {
        delete m_pGrayFrameDisplay;
        m_pGrayFrameDisplay = nullptr;
    }
    if (m_pAudioDisplay)
    {
        delete m_pAudioDisplay;
        m_pAudioDisplay = nullptr;
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
        m_thFrameReader = std::thread(&MediaReader::ReadThread, this);
    }
    if (!m_thVideoProcess.joinable())
    {
        m_thVideoProcess = std::thread(&MediaReader::VideoThread, this);
    }
    if (!m_thAudioProcess.joinable())
    {
        m_thAudioProcess = std::thread(&MediaReader::AudioThread, this);
    }
    return true;
}

bool MediaReader::Stop()
{
    return false;
}

void MediaReader::UpdateDisplaySize(int width, int height)
{
    if (m_pVideoDisplay)
        m_pVideoDisplay->UpdateDisplaySize(width, height);
}

void MediaReader::SetPlayEvent(PlayEvent* playEvent)
{
    m_playEvent = playEvent;
}

size_t MediaReader::GetAudioFrame(uint8_t* buffer, size_t len)
{
    return size_t();
}

int g_videoFrameIndex = 0;
int g_audioFrameIndex = 0;
void MediaReader::ReadThread()
{
    //FrameReaderSync();
    //return;
    m_bFrameReaderRunning = true;
    AVPacket* packet = av_packet_alloc();
    while (m_bFrameReaderRunning.load() && !m_bStreamOver.load())
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            qDebug() << "av_read_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
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
    qDebug() << "FrameReader end";
}

void MediaReader::VideoDecode(AVPacket* packet)
{
    if (packet->stream_index != m_videoStreamIndex)
        return;

    bool isKeyPacket = packet->flags & AV_PKT_FLAG_KEY;//关键帧
    int ret = avcodec_send_packet(m_videoCodecContext, packet);
    if (ret < 0)
    {//错误处理
        qDebug() << "avcodec_send_packet err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_videoCodecContext, frame);
        if (ret < 0)
        {//错误处理
            //qDebug() << "avcodec_receive_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        if (frame->key_frame == 1)
        {//关键帧
        }
        //qDebug() << "decode video frame index:" << g_videoFrameIndex;

        if (frame->format == m_hwPixFmt) {
            AVFrame* pHwFrame = av_frame_alloc();
            // 从GPU内存下载到系统内存
            int ret = av_hwframe_transfer_data(pHwFrame, frame, 0);
            if (AVERROR(ret)) {
                qDebug() << "av_hwframe_transfer_data err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                av_frame_unref(pHwFrame);
                av_frame_free(&pHwFrame);
                av_frame_unref(frame);
                av_frame_free(&frame);
                break;
            }
            frame = pHwFrame;
        }

        g_videoFrameIndex++;
        //av_frame_unref(frame);
        //av_frame_free(&frame);
        m_videoFrameQueue.Push(frame);
        //av_frame_unref(frame);//不可解引用
    }
}

void MediaReader::AudioDecode(AVPacket* packet)
{
    if (packet->stream_index != m_audioStreamIndex)
        return;

    int ret = avcodec_send_packet(m_audioCodecContext, packet);
    if (ret < 0)
    {//错误处理
        qDebug() << "avcodec_send_packet err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_audioCodecContext, frame);
        if (ret < 0)
        {//错误处理
            //qDebug() << "avcodec_receive_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            av_frame_unref(frame);
            av_frame_free(&frame);
            break;
        }
        //qDebug() << "decode audio frame index:" << g_audioFrameIndex;
        g_audioFrameIndex++;
        m_audioFrameQueue.Push(frame);
        //av_frame_unref(frame);
    }
}

void MediaReader::VideoThread()
{
    if (!m_formatContext || !m_videoCodecContext)
        return;

    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    m_bVideoProcessRunning.store(true);
    int frameIndex = 0;
    while (m_bVideoProcessRunning.load())
    {
        AVFrame* frame = m_videoFrameQueue.PopFront();
        if (!frame) {
            av_usleep(1000);
            continue;
        }

        if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
            av_frame_unref(frame);
            av_frame_free(&frame);
            continue;
        }
        bool isTooGray = IsGrayFrame(frame);
        if (isTooGray) {
            av_frame_unref(frame);
            av_frame_free(&frame);
            continue;
        }
        if (m_pVideoDisplay) {
            VideoFrame outFrame;
            outFrame.pts = frame->pts;
            outFrame.timebase = av_q2d(videoStream->time_base);
            m_pVideoDisplay->DisplayInput(frame, outFrame);
        }
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

void MediaReader::AudioThread()
{
    if (!m_formatContext || !m_videoCodecContext)
        return;

    AVStream*      audioStream = m_formatContext->streams[m_audioStreamIndex];
    const int      in_sample_rate = m_audioCodecContext->sample_rate;
    AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
    int            int_spb = av_get_bytes_per_sample(in_sfmt);
    uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
    int            in_channels = m_audioCodecContext->channels;

    m_bAudioProcessRunning.store(true);
    int frameIndex = 0;
    while (m_bAudioProcessRunning.load())
    {
        AVFrame* frame = m_audioFrameQueue.PopFront();
        if (!frame) {
            av_usleep(1000);
            continue;
        }

        frameIndex++;
        if (m_pAudioDisplay) {
            AudioFrame outFrame;
            m_pAudioDisplay->DisplayInput(frame, outFrame);
            m_masterClock = frame->pts * av_q2d(audioStream->time_base);
            if (m_playEvent)
            {
                //AudioFrame aframe;
                //aframe.audioData = frameResample->data[0];
                //aframe.dataSize = out_spb * out_channels * out_nb_samples;
                //aframe.sampleRate = m_param.outputSampleRate;
                //aframe.bitPerSample = m_param.outputBitPerSample;
                //aframe.channelCount = m_param.outputChannelCount;
                ////m_param.audioCallback(aframe);
                //m_audioFrameQueueNotFullCV.notify_all();
            }
        }
        av_frame_unref(frame);
        av_frame_free(&frame);
    }

}

void MediaReader::DisplayThread()
{
}

void MediaReader::DisplayVideo(const VideoFrame& frame)
{
    double frameDelay = (frame.pts - m_lastVideoFramePts) * frame.timebase;
    double curTime = av_gettime_relative() / 1000000.0;
    double usedTime = m_lastFrameRenderTime == 0 ? 0.0 : curTime - m_lastFrameRenderTime;
    double actualDelay = frameDelay - usedTime - m_lastDelayDelta;
    if (m_lastFrameRenderTime != 0) {
        //double pts = frame->pts * av_q2d(videoStream->time_base);
        //// 计算与音频时钟的差值
        //double diff = pts - m_masterClock;
        //// 同步阈值（可根据需要调整）
        //if (diff > m_syncThreshold) {
        //    // 视频落后，加快播放（减少延迟）
        //    delay = delay * 0.9;
        //}
        //else if (diff < -m_syncThreshold) {
        //    // 视频超前，减慢播放（增加延迟）
        //    delay = delay * 1.1;
        //}
        //// 确保延迟在合理范围内
        //delay = FFMAX(0.01, FFMIN(delay, 0.1));
        if (actualDelay > 0) {
            av_usleep(actualDelay * 1000000.0);
        }
    }
    auto displayTime = av_gettime_relative() / 1000000.0;
    if (m_playEvent)
        m_playEvent->onVideoFrame(frame);
    double delayDelta = displayTime - curTime - actualDelay;
    //qDebug() << "display time:" << (displayTime - m_lastFrameRenderTime) << ", delta:" << delayDelta << ", usedTime:" << usedTime << ", frameDelay:" << frameDelay << ", actualDelay:" << actualDelay;

    m_lastFrameRenderTime = displayTime;
    m_lastVideoFramePts = frame.pts;
    m_lastDelayDelta = delayDelta;
}

void MediaReader::FrameReaderSync()
{
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    m_bFrameReaderRunning = true;
    AVPacket* packet = av_packet_alloc();
    while (m_bFrameReaderRunning.load() && !m_bStreamOver.load())
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            qDebug() << "av_read_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            if (ret == AVERROR_EOF)
            {
                m_bStreamOver = true;
                qDebug() << "FrameReader stream over";
            }
            break;
        }
        if (packet->stream_index == m_videoStreamIndex)
        {
            ret = avcodec_send_packet(m_videoCodecContext, packet);
            if (ret < 0)
            {//错误处理
                qDebug() << "avcodec_send_packet err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                continue;
            }
            while (true)
            {
                AVFrame* frame = nullptr;
                AVFrame* hwFrame = av_frame_alloc();
                AVFrame* tempFrame = av_frame_alloc();
                ret = avcodec_receive_frame(m_videoCodecContext, tempFrame);
                if (ret < 0)
                {//错误处理
                    qDebug() << "avcodec_receive_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                    av_frame_free(&hwFrame);
                    av_frame_free(&tempFrame);
                    break;
                }
                if (tempFrame->format == m_hwPixFmt) {
                    // 从GPU内存下载到系统内存
                    ret = av_hwframe_transfer_data(hwFrame, tempFrame, 0);
                    if (AVERROR(ret)) {
                        qDebug() << "av_hwframe_transfer_data err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                        av_frame_free(&hwFrame);
                        av_frame_free(&tempFrame);
                        break;
                    }
                    frame = hwFrame;
                    frame->pts = tempFrame->pts;
                } else {
                    frame = tempFrame;
                }
                qDebug() << "decode video frame index:" << g_videoFrameIndex;
                g_videoFrameIndex++;
                if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
                    av_frame_free(&hwFrame);
                    av_frame_free(&tempFrame);
                    continue;
                }
                bool isTooGray = IsGrayFrame(frame);
                if (isTooGray) {
                    av_frame_unref(frame);
                    av_frame_free(&frame);
                    continue;
                }
                if (m_pVideoDisplay) {
                    VideoFrame outFrame;
                    m_pVideoDisplay->DisplayInput(frame, outFrame);
                    double frameDelay = (frame->pts - m_lastVideoFramePts) * av_q2d(videoStream->time_base);
                    double curTime = av_gettime_relative() / 1000000.0;
                    double usedTime = m_lastFrameRenderTime == 0 ? 0.0 : curTime - m_lastFrameRenderTime;
                    double actualDelay = frameDelay - usedTime - m_lastDelayDelta;
                    if (m_lastFrameRenderTime != 0) {
                        //double pts = frame->pts * av_q2d(videoStream->time_base);
                        //// 计算与音频时钟的差值
                        //double diff = pts - m_masterClock;
                        //// 同步阈值（可根据需要调整）
                        //if (diff > m_syncThreshold) {
                        //    // 视频落后，加快播放（减少延迟）
                        //    delay = delay * 0.9;
                        //}
                        //else if (diff < -m_syncThreshold) {
                        //    // 视频超前，减慢播放（增加延迟）
                        //    delay = delay * 1.1;
                        //}
                        //// 确保延迟在合理范围内
                        //delay = FFMAX(0.01, FFMIN(delay, 0.1));
                        if (actualDelay > 0) {
                            av_usleep(actualDelay * 1000000.0);
                        }
                    }
                    auto displayTime = av_gettime_relative() / 1000000.0;
                    if (m_playEvent)
                        m_playEvent->onVideoFrame(outFrame);
                    double delayDelta = displayTime - curTime - actualDelay;
                    qDebug() << "display time:" << (displayTime - m_lastFrameRenderTime) << ", delta:" << delayDelta << ", usedTime:" << usedTime << ", frameDelay:" << frameDelay << ", actualDelay:" << actualDelay;

                    m_lastFrameRenderTime = displayTime;
                    m_lastVideoFramePts = frame->pts;
                    m_lastDelayDelta = delayDelta;
                }
                av_frame_unref(frame);
                av_frame_free(&frame);
            }
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {
            //AudioDecode(packet);
        }
    }
    av_packet_free(&packet);
    qDebug() << "FrameReader end";
}

bool MediaReader::IsGrayFrame(AVFrame* pFrame)
{
    return false;
    if (pFrame->width <= 0 || pFrame->height <= 0)
        return false;

    int iWidth = pFrame->width;
    int iHeight = pFrame->height;
    int gcd = GetGCD(iWidth, iHeight);
    if (gcd > 0) {
        iWidth = iWidth / gcd;
        iHeight = iHeight / gcd;
        //获取一个宽高都比100大的，比例一致的宽高，宽高比例要是不一致，重采样出来的图片是混乱的
        while (iWidth <= 100 && iHeight <= 100)
        {
            iWidth *= 2;
            iHeight *= 2;
        }
    }

    if (!m_pGrayFrameDisplay) {
        VideoSpec spec;
        spec.width = iWidth;
        spec.height = iHeight;
        spec.format = 8;//AV_PIX_FMT_GRAY8
        m_pGrayFrameDisplay = new VideoDisplay(0, spec);
    }
    VideoFrame outFrame;
    auto err = m_pGrayFrameDisplay->DisplayInput(pFrame, outFrame);
    if (err.code != PlayErrorCode::kNoError)
        return false;

    try {//尝试捕捉opencv的错误

        // 将帧转换为灰度图像
        cv::Mat grayFrame = cv::Mat(outFrame.spec.height, outFrame.spec.width, CV_8UC1, outFrame.data, outFrame.size);
        //检查灰度可以压缩图像大小，以加速检查
        //cv::resize(matRgb,grayFrame,cv::Size(100,100));
        //cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);

        // 计算灰度图像的方差
        cv::Scalar mean, stddev;
        cv::meanStdDev(grayFrame, mean, stddev);
        double dGrayScaleDegree = stddev[0] * stddev[0];
        if (dGrayScaleDegree < 100.0) {
            // cv::imshow("gray",matRgb);
            // cv::waitKey(1);
            return true;
        }
    }
    catch (const std::bad_alloc& e) {
        // 捕获内存分配失败的异常
        qCritical() << "Memory allocation failed: " << e.what();
    }
    catch (const cv::Exception& e) {
        // 捕获 OpenCV 相关的异常
        qCritical() << "OpenCV error: " << e.what();
    }
    catch (...) {
        // 捕获其他类型的异常
        qCritical() << "An unexpected error occurred.";
    }
    //cv::Mat mat = cv::Mat(frame->height, frame->width, CV_8UC3, frameRGB->data[0], frameRGB->linesize[0]);
    //bool isTooGray = false;
    //{
    //    // 将帧转换为灰度图像
    //    cv::Mat grayFrame;
    //    //检查灰度可以压缩图像大小，以加速检查
    //    cv::resize(mat, grayFrame, cv::Size(100, 100));
    //    cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);
    //    // 计算灰度图像的方差
    //    cv::Scalar mean, stddev;
    //    cv::meanStdDev(grayFrame, mean, stddev);
    //    double dGrayScaleDegree = stddev[0] * stddev[0];
    //    if (dGrayScaleDegree < 100.0) {
    //        isTooGray = true;
    //    }
    //}
    return false;
}

