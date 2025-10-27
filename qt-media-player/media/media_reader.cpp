#include "media_reader.h"
#include <QDebug>
#include "utils/jitter_buffer.h"
#include "media/media_display.h"
#include "media/media_utils.h"

#ifdef DEBUG_VIDEO_FRAME_WRITE

#endif // DEBUG_VIDEO_FRAME_WRITE

#ifdef DEBUG_AUDIO_RESAMPLE_WRITE

#endif // DEBUG_AUDIO_RESAMPLE_WRITE

/**
* 较新版本的av_packet_free、av_frame_free内部都先执行了av_packet_unref、av_frame_unref，因此没必要再多调一次
*/
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
    avformat_network_init();
}

MediaReader::~MediaReader()
{
    avformat_network_deinit();
}

const AVPixelFormat& MediaReader::GetHwPixFmt() const
{
    return m_hwPixFmt;
}

void MediaReader::QuitHwDecode()
{
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
}

//复用同一个AVHWDeviceContext能减少大量cpu和gpu
static AVBufferRef* g_d3d11_device = nullptr;
static AVBufferRef* g_dxva2_device = nullptr;

bool MediaReader::StreamOpen()
{
    qDebug() << "open url:" << QString::fromStdString(m_param.url);
    //配置该流的ffmpeg设置
    AVDictionary* pOptDict = NULL;
    av_dict_set(&pOptDict, "stimeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间，可能不生效
    av_dict_set(&pOptDict, "timeout", "5000000", 0);//适应延迟网络，设置5s的等待链接时间
    av_dict_set(&pOptDict, "buffer_size", "8192000", 0);//控制解码器或编码器的内部缓冲区大小,配置8M缓冲以适应高分辨率视频
    av_dict_set(&pOptDict, "recv_buffer_size", "4096000", 0);     // 防止花屏, max 4M.:用于控制网络接收缓冲区大小，适用于高带宽或高延迟的网络环境
    av_dict_set(&pOptDict, "tune", "stillimage,fastdecode,zerolatency", 0);//优化静态图像编码,快速解码和低延时传输
    av_dict_set(&pOptDict, "rtsp_transport", "tcp", 0);//tcp拉流，尽量保证不丢包
    //av_dict_set(&pOptDict, "rtbufsize", "20M", 0);
    //av_dict_set(&pOptDict, "buffer_size", "1024000", 0);
    int ret = avformat_open_input(&m_formatContext, m_param.url.c_str(), nullptr, &pOptDict);
    av_dict_free(&pOptDict);
    pOptDict = nullptr;

    // 打开流
    if (AVERROR(ret))
    {
        qDebug() << "avformat_open_input failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
        return false;
    }

    // 获取流信息
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0)
    {
        qDebug() << "avformat_find_stream_info failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
        avformat_close_input(&m_formatContext);
        return false;
    }

    // 寻找视频流和音频流，这里只取一次
    int videoStreamIndex = -1;
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatContext->nb_streams; ++i)
    {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIndex < 0)
        {
            videoStreamIndex = i;
        }
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIndex < 0)
        {
            audioStreamIndex = i;
        }
    }

    if (videoStreamIndex < 0 && audioStreamIndex < 0)
    {
        qDebug() << "find video stream and audio stream failed";
        avformat_close_input(&m_formatContext);
        return false;
    }

    if (videoStreamIndex >= 0)
    {
        // 获取视频流解码器参数
        AVCodecParameters* videoCodecParameters = m_formatContext->streams[videoStreamIndex]->codecpar;
        // 获取视频解码器
        const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
        // 创建解码器上下文
        m_videoCodecContext = avcodec_alloc_context3(videoCodec);
        if (avcodec_parameters_to_context(m_videoCodecContext, videoCodecParameters) < 0)
        {
            qDebug() << "avcodec_parameters_to_context failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
        //m_videoCodecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
        m_videoCodecContext->opaque = this; //用于回调里获取的指针
        m_videoCodecContext->thread_count = 1;

        if (!m_param.hwDeviceName.empty()) {
            m_hwDeviceType = av_hwdevice_find_type_by_name(m_param.hwDeviceName.c_str());
            if (m_hwDeviceType == AV_HWDEVICE_TYPE_NONE) {
                qDebug() << "device type is not supported:" << QString::fromStdString(m_param.hwDeviceName);
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
            if (m_hwDeviceType == AV_HWDEVICE_TYPE_DXVA2) {
                if (!g_dxva2_device) {
                    ret = av_hwdevice_ctx_create(&g_dxva2_device, m_hwDeviceType, nullptr, nullptr, 0);
                    if (AVERROR(ret)) {
                        qDebug() << "av_hwdevice_ctx_create failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                        m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                    }
                }
                m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_dxva2_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
            }
            else if (m_hwDeviceType == AV_HWDEVICE_TYPE_D3D11VA) {
                if (!g_d3d11_device) {
                    ret = av_hwdevice_ctx_create(&g_d3d11_device, m_hwDeviceType, nullptr, nullptr, 0);
                    if (AVERROR(ret)) {
                        qDebug() << "av_hwdevice_ctx_create failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                        m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                    }
                }
                m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_d3d11_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
            }
            m_videoCodecContext->get_format = GetHwFormat;
        }

        ret = avcodec_open2(m_videoCodecContext, videoCodec, nullptr);
        if (ret != 0) {
            qDebug() << "avcodec_open2 failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }

        if (m_videoCodecContext->width <= 0 || m_videoCodecContext->height <= 0 || m_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
        {
            qDebug() << "codecContext data error";
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
    }
    m_videoStreamIndex = videoStreamIndex;
    if (audioStreamIndex >= 0)
    {
        // 获取音频流解码器参数
        AVCodecParameters* audiooCodecParameters = m_formatContext->streams[audioStreamIndex]->codecpar;
        // 获取视频解码器
        const AVCodec* audioCodec = avcodec_find_decoder(audiooCodecParameters->codec_id);
        // 创建解码器上下文
        m_audioCodecContext = avcodec_alloc_context3(audioCodec);
        ret = avcodec_parameters_to_context(m_audioCodecContext, audiooCodecParameters);
        if (AVERROR(ret)) {
            qDebug() << "avcodec_parameters_to_context failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
        ret = avcodec_open2(m_audioCodecContext, audioCodec, nullptr);
        if (ret != 0) {
            qDebug() << "avcodec_open2 failed," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
    }
    m_audioStreamIndex = audioStreamIndex;
    return true;
}

void MediaReader::StreamClose()
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
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
}

bool MediaReader::Play(const MediaParameter& param)
{
    m_param = param;
    m_bThreadRun = true;
    if (!m_thReader.joinable())
    {
        m_thReader = std::thread(&MediaReader::ReadThread, this);
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

bool MediaReader::Stop()
{
    m_bThreadRun = false;

    if (m_thVideoDecoder.joinable()) {
        m_thVideoDecoder.join();
    }
    if (m_thAudioDecoder.joinable()) {
        m_thAudioDecoder.join();
    }
    if (m_thReader.joinable()) {
        m_thReader.join();
    }
    StreamClose();
    return true;
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
    StreamOpen();

    FrameReaderSync();
    return;
    if (m_videoStreamIndex >= 0) {
        if (!m_thVideoDecoder.joinable())
        {
            m_thVideoDecoder = std::thread(&MediaReader::VideoThread, this);
        }
    }
    if (m_audioStreamIndex >= 0) {
        if (!m_thAudioDecoder.joinable())
        {
            m_thAudioDecoder = std::thread(&MediaReader::AudioThread, this);
        }
    }

    AVPacket* packet = av_packet_alloc();
    while (m_bThreadRun.load() && !m_bStreamOver.load())
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
            m_videoPacketQueue.Push(packet);
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {
            m_audioPacketQueue.Push(packet);
        }

    }
    av_packet_free(&packet);
    qDebug() << "FrameReader end";
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

    int frameIndex = 0;
    while (m_bThreadRun.load() && !m_bStreamOver.load())
    {
        av_usleep(1000);
        AVPacket* packet = m_videoPacketQueue.PopFront();
        if (!packet)
            continue;

        VideoDecode(packet);
    }
}

void MediaReader::AudioThread()
{
    if (!m_formatContext || !m_videoCodecContext)
        return;

    AVStream* audioStream = m_formatContext->streams[m_audioStreamIndex];
    const int      in_sample_rate = m_audioCodecContext->sample_rate;
    AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
    int            int_spb = av_get_bytes_per_sample(in_sfmt);
    uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
    int            in_channels = m_audioCodecContext->channels;

    while (m_bThreadRun.load() && !m_bStreamOver.load())
    {
        av_usleep(1000);
        AVPacket* packet = m_audioPacketQueue.PopFront();
        if (!packet)
            continue;

        AudioDecode(packet);
    }

}

static int64_t m_i64FirstIFramePts = 0;
static int64_t m_i64FirstIFrameTs = 0;
static int64_t m_iThrowPacketCount = 0;
static bool m_isThrowPacket = false;

void MediaReader::VideoDecode(AVPacket* packet)
{
    if (packet->stream_index != m_videoStreamIndex)
        return;

    //bool isKeyPkt = packet->flags & AV_PKT_FLAG_KEY;//关键帧
    //int64_t curPts = packet->pts;
    //int64_t curTs = av_gettime_relative();//微秒
    //if (isKeyPkt) {
    //    if (curPts >= 0 && curTs >= 0) {
    //        if (m_i64FirstIFramePts == 0 && m_i64FirstIFrameTs == 0) {
    //            m_i64FirstIFramePts = curPts;
    //            m_i64FirstIFrameTs = curTs;
    //        }
    //        else {
    //            //微秒
    //            int64_t intervalTime = curTs - m_i64FirstIFrameTs;//间隔时间
    //            int64_t pktIntervalTime = (curPts - m_i64FirstIFramePts) * av_q2d(m_formatContext->streams[m_videoStreamIndex]->time_base) * 1000000.0;//包间隔
    //            if (intervalTime - pktIntervalTime > 2000000) {
    //                m_isThrowPacket = true;
    //            }
    //            else {
    //                m_isThrowPacket = false;
    //            }
    //            qDebug() << "m_isThrowPacket:" << m_isThrowPacket;
    //        }

    //    }
    //}

    //if (!isKeyPkt && m_isThrowPacket) {
    //    m_iThrowPacketCount++;
    //    return;
    //}

    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
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
                av_frame_free(&pHwFrame);
                av_frame_free(&frame);
                break;
            }
            frame = pHwFrame;
        }
        g_videoFrameIndex++;
        if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
            av_frame_free(&frame);
            continue;
        }
        bool isTooGray = IsGrayFrame(frame);
        if (isTooGray) {
            av_frame_free(&frame);
            continue;
        }
        if (m_pVideoDisplay) {
            VideoFrame outFrame;
            outFrame.pts = frame->pts;
            outFrame.timebase = av_q2d(videoStream->time_base);
            m_pVideoDisplay->DisplayInput(frame, outFrame);
        }
        av_frame_free(&frame);
    }
}

void MediaReader::AudioDecode(AVPacket* packet)
{
    if (packet->stream_index != m_audioStreamIndex)
        return;

    AVStream* audioStream = m_formatContext->streams[m_audioStreamIndex];
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
            av_frame_free(&frame);
            break;
        }
        //qDebug() << "decode audio frame index:" << g_audioFrameIndex;
        g_audioFrameIndex++;

        if (m_pAudioDisplay) {
            AudioFrame outFrame;
            //m_pAudioDisplay->DisplayInput(frame, outFrame);
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
    if (!m_formatContext || m_videoStreamIndex < 0)
        return;
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timeBase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    m_bThreadRun = true;
    AVPacket* packet = av_packet_alloc();
    while (m_bThreadRun.load() && !m_bStreamOver.load())
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
                    //qDebug() << "avcodec_receive_frame err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                    av_frame_free(&hwFrame);
                    av_frame_free(&tempFrame);
                    break;
                }
                if (tempFrame->format == m_hwPixFmt) {
                    // 从GPU内存下载到cpu内存
                    // av_hwframe_map效率比av_hwframe_transfer_data高，耗时减少约1/3，但是性能好像并没有优化多少，cpu降低了，但是gpu变高了
                    // sw_frame->format 现在是 AV_PIX_FMT_DRM_PRIME / CUDA / D3D11 等
                    ret = av_hwframe_map(hwFrame, tempFrame, AV_HWFRAME_MAP_READ);
                    if (AVERROR(ret)) {
                        ret = av_hwframe_transfer_data(hwFrame, tempFrame, 0);
                        if (AVERROR(ret)) {
                            qDebug() << "av_hwframe_transfer_data err," << QString("%1:%2").arg(ret).arg(av_error_qstring(ret));
                            av_frame_free(&hwFrame);
                            av_frame_free(&tempFrame);
                            break;
                        }
                    }
                    frame = hwFrame;
                    frame->width = tempFrame->width;
                    frame->height = tempFrame->height;
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
                    av_frame_free(&hwFrame);
                    av_frame_free(&tempFrame);
                    continue;
                }
                if (m_pVideoDisplay) {
                    VideoFrame outFrame;
                    m_pVideoDisplay->DisplayInput(frame, outFrame);
                }
                av_frame_free(&hwFrame);
                av_frame_free(&tempFrame);
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
        cv::Mat grayFrame = cv::Mat(outFrame.spec.height, outFrame.spec.width, CV_8UC1, outFrame.data);
        //检查灰度可以压缩图像大小，以加速检查
        //cv::resize(matRgb,grayFrame,cv::Size(100,100));
        //cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);

        //static int frameIndex = 0;
        //// 保存图片
        //char filename[100];
        //sprintf(filename, "E:\\code\\media\\temp\\%d.jpg", frameIndex);
        //cv::imwrite(filename, grayFrame);
        //frameIndex++;
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

