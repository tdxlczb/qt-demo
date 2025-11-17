#include "media_reader.h"
#include "log.h"
#include "media_utils.h"

#define PLAYTAG  std::to_string(m_playIndex) + " "  
/**
* 较新版本的av_packet_free、av_frame_free内部都先执行了av_packet_unref、av_frame_unref，因此没必要再多调一次
*/
static AVPixelFormat GetHwFormat(AVCodecContext* pCodecContext, const enum AVPixelFormat* pPixFmts)
{
    const enum AVPixelFormat* pPixFmt;
    MediaReader* pReader = (MediaReader*)pCodecContext->opaque;
    if (!pReader)
        return AV_PIX_FMT_NONE;
    //遍历给定的AVPixelFormat数组中存不存在AVCodecContext支持的硬解码格式
    for (pPixFmt = pPixFmts; *pPixFmt != AV_PIX_FMT_NONE; ++pPixFmt)
    {
        if (*pPixFmt == pReader->GetHwPixFmt())
        {
            return *pPixFmt;
        }
    }
    LOG_WARN << pReader->GetPlayIndex() << " HwPixFmt not found " << pReader->GetHwPixFmt();
    pReader->QuitHwDecode();
    return AV_PIX_FMT_NONE;
}

MediaReader::MediaReader(int index)
    : m_uid(uuid32())
    , m_playIndex(index)
{
    avformat_network_init();
}

MediaReader::~MediaReader()
{
    avformat_network_deinit();
}

std::string MediaReader::GetId() const
{
    return m_uid;
}

int MediaReader::GetPlayIndex() const
{
    return m_playIndex;
}

std::string MediaReader::GetPlayUrl() const
{
    return m_url;
}

void MediaReader::Play(const std::string& url, const PlayOptions& options)
{
    Stop();
    m_url = url;
    m_options = options;

    m_isThreadRun.store(true);
    if (!m_thReader.joinable())
    {
        m_thReader = std::thread(&MediaReader::ReadThread, this);
    }
}

void MediaReader::Stop()
{
    m_url = "";
    m_options = PlayOptions{};

    m_isThreadRun.store(false);
    if (m_thVideoDisplay.joinable()) {
        m_thVideoDisplay.join();
    }
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

    m_isStreamOver.store(false);
    m_videoPacketQueue.Clear();
    m_audioPacketQueue.Clear();
    m_videoFrameQueue.Clear();

    m_videoPacketIndex = 0;
    m_audioPacketIndex = 0;
    m_videoFrameIndex = 0;
    m_audioFrameIndex = 0;
    m_iLastCountTime = 0;
    m_clockStart = 0.0;
    m_startPts = 0.0;
}

void MediaReader::SetPlayEvent(PlayEvent* playEvent)
{
    m_playEvent = playEvent;
}

const AVPixelFormat& MediaReader::GetHwPixFmt() const
{
    return m_hwPixFmt;
}

void MediaReader::QuitHwDecode()
{
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
}

void MediaReader::ReadThread()
{
    if (!StreamOpen()) {
        LOG_ERROR << PLAYTAG << "stream open failed";
        return;
    }

    //FrameReaderSync();
    //return;
    if (m_videoStreamIndex >= 0) {
        if (!m_thVideoDecoder.joinable()) {
            m_thVideoDecoder = std::thread(&MediaReader::VideoThread, this);
        }
        if (m_isAsyncDisplay && !m_thVideoDisplay.joinable()) {
            m_thVideoDisplay = std::thread(&MediaReader::DisplayThread, this);
        }
    }
    if (m_audioStreamIndex >= 0) {
        if (!m_thAudioDecoder.joinable()) {
            m_thAudioDecoder = std::thread(&MediaReader::AudioThread, this);
        }
    }
    LOG_INFO << PLAYTAG << "FrameReader start";
    AVPacket* packet = av_packet_alloc();
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            LOG_ERROR << PLAYTAG << "av_read_frame err," << ret << ":" << av_error_string(ret);
            if (ret == AVERROR_EOF)
            {
                m_isStreamOver = true;
                LOG_INFO << PLAYTAG << "FrameReader stream over";
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
        int64_t curTime = av_gettime_relative();
        if (packet->stream_index == m_videoStreamIndex)
        {
            m_videoPacketSize += packet->size;
            m_videoPacketQueue.Push(packet, !m_isMediaFile);
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {
            m_audioPacketSize += packet->size;
            m_audioPacketQueue.Push(packet, !m_isMediaFile);
        }
        if (curTime - m_lastCountPacketSizeTime >= 1000000) {
            m_lastCountPacketSizeTime = curTime;
            int videoKbps = (double)m_videoPacketSize * 8 / 1000;
            int audioKbps = (double)m_audioPacketSize * 8 / 1000;
            LOG_INFO << PLAYTAG << "video " << videoKbps << ", audio " << audioKbps;
            m_videoPacketSize = 0;
            m_audioPacketSize = 0;
        }
    }
    av_packet_free(&packet);
    LOG_INFO << PLAYTAG << "FrameReader end";
}

void MediaReader::VideoThread()
{
    if (!m_formatContext || !m_videoCodecContext)
        return;

    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timebase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    int frameIndex = 0;
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        av_usleep(1000);
        AVPacket* packet = m_videoPacketQueue.PopFront();
        if (!packet)
            continue;

        m_videoPacketIndex++;
        VideoDecode(packet);
        av_packet_free(&packet);
    }
}

void MediaReader::AudioThread()
{
    if (!m_formatContext || !m_videoCodecContext)
        return;

    //AVStream* audioStream = m_formatContext->streams[m_audioStreamIndex];
    //const int      in_sample_rate = m_audioCodecContext->sample_rate;
    //AVSampleFormat in_sfmt = m_audioCodecContext->sample_fmt;
    //int            int_spb = av_get_bytes_per_sample(in_sfmt);
    //uint64_t       in_channel_layout = m_audioCodecContext->channel_layout;
    //int            in_channels = m_audioCodecContext->channels;

    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        av_usleep(1000);
        AVPacket* packet = m_audioPacketQueue.PopFront();
        if (!packet)
            continue;

        m_audioPacketIndex++;
        AudioDecode(packet);
        av_packet_free(&packet);
    }
}

void MediaReader::VideoDecode(AVPacket* packet)
{
    if (packet->stream_index != m_videoStreamIndex)
        return;

    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    int64_t t1 = av_gettime_relative();
    int ret = avcodec_send_packet(m_videoCodecContext, packet);
    int64_t t2 = av_gettime_relative();
    if (ret < 0)
    {//错误处理
        LOG_ERROR << PLAYTAG << "avcodec_send_packet err," << ret << ":" << av_error_string(ret);
        return;
    }
    if (t2 - t1 > 30000) {
        LOG_INFO << PLAYTAG << "packet index:" << m_videoPacketIndex << ", avcodec_send_packet timeout " << (t2 - t1) / 1000;
    }

    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        int64_t t3 = av_gettime_relative();
        ret = avcodec_receive_frame(m_videoCodecContext, frame);
        int64_t t4 = av_gettime_relative();
        if (ret < 0)
        {//错误处理
            av_frame_free(&frame);
            break;
        }
        if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
            av_frame_free(&frame);
            continue;
        }

        m_videoFrameIndex++;
        if (t4 - t3 > 10000) {
            LOG_INFO << PLAYTAG << "frame index:" << m_videoFrameIndex << ", avcodec_receive_frame timeout " << (t4 - t3) / 1000;
        }
        if (frame->format == m_hwPixFmt) {
            // 从GPU内存下载到cpu内存
            // av_hwframe_map效率比av_hwframe_transfer_data高，耗时减少约1/3，但是性能好像并没有优化多少，cpu降低了，但是gpu变高了
            // sw_frame->format 现在是 AV_PIX_FMT_DRM_PRIME / CUDA / D3D11 等
            AVFrame* hwFrame = av_frame_alloc();
            ret = av_hwframe_map(hwFrame, frame, AV_HWFRAME_MAP_READ);
            if (AVERROR(ret)) {
                ret = av_hwframe_transfer_data(hwFrame, frame, 0);
                if (AVERROR(ret)) {
                    LOG_ERROR << PLAYTAG << "av_hwframe_transfer_data err," << ret << ":" << av_error_string(ret);
                    av_frame_free(&hwFrame);
                    av_frame_free(&frame);
                    break;
                }
            }
            hwFrame->width = frame->width;
            hwFrame->height = frame->height;
            hwFrame->pts = frame->pts;
            av_frame_free(&frame);
            frame = hwFrame;
        }

        //LOG_DEBUG << PLAYTAG << "decode video frame index:" << m_videoFrameIndex;
        int64_t curTime = av_gettime_relative();
        if (curTime - m_iLastCountTime > 4000000) {
            LOG_INFO << PLAYTAG << "frame index:" << m_videoFrameIndex;
            m_iLastCountTime = curTime;
        }
        if (m_isAsyncDisplay) {
            m_videoFrameQueue.Push(frame);
        }
        else {
            DisplayVideo(frame);
            av_frame_free(&frame);
        }
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
        LOG_ERROR << PLAYTAG << "avcodec_send_packet err," << ret << ":" << av_error_string(ret);
        return;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_audioCodecContext, frame);
        if (ret < 0)
        {//错误处理
            av_frame_free(&frame);
            break;
        }
        //LOG_DEBUG << PLAYTAG << "decode audio frame index:" << m_audioFrameIndex;
        m_audioFrameIndex++;
        DisplayAudio(frame);
        av_frame_free(&frame);
    }
}

void MediaReader::DisplayThread()
{
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        av_usleep(1000);
        AVFrame* frame = m_videoFrameQueue.PopFront();
        if (!frame)
            continue;

        DisplayVideo(frame);
        av_frame_free(&frame);
    }
}

void MediaReader::DisplayVideo(AVFrame* frame)
{
    int npts = frame->pts == AV_NOPTS_VALUE ? 0 : frame->pts;
    //LOG_INFO << PLAYTAG << "frameIndex:" << m_videoFrameIndex << ",pts:" << npts;
    double pts = npts * av_q2d(m_formatContext->streams[m_videoStreamIndex]->time_base);
    double now = av_gettime_relative() / 1000000.0;
    if (m_clockStart <= 0.0) {
        m_clockStart = now;
        m_startPts = pts;
        LOG_INFO << PLAYTAG << "first pts:" << npts;
    }
    //时钟同步
    double elapsedPts = pts - m_startPts; // 相对 pts
    double elapsedClock = now - m_clockStart; // 相对 clock
    double diff = elapsedPts - elapsedClock;// +1.0; // > 0 表示视频超前, 增加1s延迟，会导致第一帧播放慢
    double delay = diff;
    if (diff > m_syncThreshold) {
        // 视频超前，减慢播放（增加延迟）
        //delay = delay * 1.1;
    }
    else if (diff < -m_syncThreshold) {
        // 视频落后，加快播放（减少延迟）,或者丢帧
        //delay = delay * 0.9;
        //return;
    }
    if (delay > 0) {
        // 确保延迟在合理范围内
        //delay = FFMAX(0.01, FFMIN(delay, 0.1));
        av_usleep(delay * 1000000.0);
    }

    if (m_playEvent) {
        VideoFrame outFrame;
        for (size_t i = 0; i < 8; i++)
        {
            outFrame.linedata[i] = frame->data[i];
            outFrame.linesize[i] = frame->linesize[i];
        }
        outFrame.pts = pts;
        outFrame.index = m_videoFrameIndex;
        outFrame.spec.width = frame->width;
        outFrame.spec.height = frame->height;
        outFrame.spec.format = frame->format;
        m_playEvent->onVideoFrame(outFrame);
    }
}

void MediaReader::DisplayAudio(AVFrame* frame)
{
    int npts = frame->pts == AV_NOPTS_VALUE ? 0 : frame->pts;
    //LOG_INFO << PLAYTAG << "frameIndex:" << m_audioFrameIndex << ",pts:" << npts;
    double pts = npts * av_q2d(m_formatContext->streams[m_audioStreamIndex]->time_base);
    if (m_playEvent) {
        AudioFrame outFrame;
        for (size_t i = 0; i < 8; i++)
        {
            outFrame.linedata[i] = frame->data[i];
            outFrame.linesize[i] = frame->linesize[i];
        }
        outFrame.pts = pts;
        outFrame.index = m_audioFrameIndex;
        outFrame.spec.sampleRate = frame->sample_rate;
        outFrame.spec.channels = frame->channels;
        outFrame.spec.format = frame->format;
        m_playEvent->onAudioFrame(outFrame);
    }
}

//复用同一个AVHWDeviceContext能减少大量cpu和gpu
static AVBufferRef* g_d3d11_device = nullptr;
static AVBufferRef* g_dxva2_device = nullptr;

bool IsNetworkStream(const std::string& url) {
    const std::vector<std::string> protocols = {
        "http://", "https://", "rtmp://", "rtsp://", "ftp://",
        "udp://", "tcp://", "mms://", "rtp://"
    };

    for (const auto& proto : protocols) {
        if (url.find(proto) == 0) {  // 以协议头开头
            return true;
        }
    }
    return false;
}

bool IsMediaTypeFile(AVFormatContext* fmtCtx, const std::string& url) {
    // 方法1: 优先检查URL协议（最准确）
    if (IsNetworkStream(url)) {
        return false;
    }

    if (!fmtCtx)
        return true;

    // 方法2: 检查iformat和flags
    if (fmtCtx->iformat) {
        // 检查是否是常见的直播格式
        const char* formatName = fmtCtx->iformat->name;
        if (strstr(formatName, "rtsp") || strstr(formatName, "rtmp")) {
            return false;
        }

        // 检查AVFMT_NOFILE标志
        if (fmtCtx->iformat->flags & AVFMT_NOFILE) {
            return false;
        }
    }

    // 方法3: 检查时长（辅助判断）
    if (fmtCtx->duration == AV_NOPTS_VALUE) {
        // 无时长，很可能是实时流
        return false;
    }

    // 默认认为是文件
    return true;
}

bool MediaReader::StreamOpen()
{
    LOG_INFO << PLAYTAG << "open url:" << m_url;
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
    int ret = avformat_open_input(&m_formatContext, m_url.c_str(), nullptr, &pOptDict);
    av_dict_free(&pOptDict);
    pOptDict = nullptr;

    // 打开流
    if (AVERROR(ret))
    {
        LOG_ERROR << PLAYTAG << "avformat_open_input failed," << ret << ":" << av_error_string(ret);
        return false;
    }

    // 获取流信息
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret < 0)
    {
        LOG_ERROR << PLAYTAG << "avformat_find_stream_info failed," << ret << ":" << av_error_string(ret);
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
        LOG_ERROR << PLAYTAG << "find video stream and audio stream failed";
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
            LOG_ERROR << PLAYTAG << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
        //m_videoCodecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
        m_videoCodecContext->opaque = this; //用于回调里获取的指针
        m_videoCodecContext->thread_count = 0;

        if (!m_options.hwdevice.empty()) {
            m_hwDeviceType = av_hwdevice_find_type_by_name(m_options.hwdevice.c_str());
            if (m_hwDeviceType == AV_HWDEVICE_TYPE_NONE) {
                LOG_ERROR << PLAYTAG << "device type is not supported:" << m_options.hwdevice;
                return false;
            }
            //查找硬解码器
            for (int i = 0;; ++i) {
                const AVCodecHWConfig* codecHWConfig = avcodec_get_hw_config(videoCodec, i);
                if (!codecHWConfig) {
                    //没有硬解码器了
                    LOG_WARN << PLAYTAG << string_format("decoder %s is not support device type:%s", videoCodec->name, av_hwdevice_get_type_name(m_hwDeviceType));
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
                        LOG_WARN << PLAYTAG << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                        m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                    }
                }
                m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_dxva2_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
            }
            else if (m_hwDeviceType == AV_HWDEVICE_TYPE_D3D11VA) {
                if (!g_d3d11_device) {
                    ret = av_hwdevice_ctx_create(&g_d3d11_device, m_hwDeviceType, nullptr, nullptr, 0);
                    if (AVERROR(ret)) {
                        LOG_WARN << PLAYTAG << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                        m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                    }
                }
                m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_d3d11_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
            }
            m_videoCodecContext->get_format = GetHwFormat;
        }

        ret = avcodec_open2(m_videoCodecContext, videoCodec, nullptr);
        if (ret != 0) {
            LOG_ERROR << PLAYTAG << "video avcodec_open2 failed," << ret << ":" << av_error_string(ret);
            avcodec_free_context(&m_videoCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }

        if (m_videoCodecContext->width <= 0 || m_videoCodecContext->height <= 0 || m_videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)
        {
            LOG_ERROR << PLAYTAG << "codecContext data error";
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
            LOG_ERROR << PLAYTAG << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
        ret = avcodec_open2(m_audioCodecContext, audioCodec, nullptr);
        if (ret != 0) {
            LOG_ERROR << PLAYTAG << "audio avcodec_open2 failed," << ret << ":" << av_error_string(ret);
            avcodec_free_context(&m_audioCodecContext);
            avformat_close_input(&m_formatContext);
            return false;
        }
    }
    m_audioStreamIndex = audioStreamIndex;

    m_isMediaFile = IsMediaTypeFile(m_formatContext, m_url);
    return true;
}

void MediaReader::StreamClose()
{
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
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
}

void MediaReader::FrameReaderSync()
{
    if (!m_formatContext || m_videoStreamIndex < 0)
        return;
    AVStream* videoStream = m_formatContext->streams[m_videoStreamIndex];
    auto      timebase = videoStream->time_base;
    double    fps = av_q2d(videoStream->avg_frame_rate);
    auto      frameCount = videoStream->nb_frames;
    int       gopSize = m_videoCodecContext->gop_size;

    LOG_INFO << PLAYTAG << "FrameReader start";
    int videoFrameIndex = 0;
    AVPacket* packet = av_packet_alloc();
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        av_packet_unref(packet);
        int ret = av_read_frame(m_formatContext, packet);
        if (ret < 0)
        {
            LOG_ERROR << PLAYTAG << "av_read_frame err," << ret << ":" << av_error_string(ret);
            if (ret == AVERROR_EOF)
            {
                m_isStreamOver.store(true);
                LOG_INFO << PLAYTAG << "FrameReader stream over";
            }
            break;
        }
        if (packet->stream_index == m_videoStreamIndex)
        {
            VideoDecode(packet);
        }
        else if (packet->stream_index == m_audioStreamIndex)
        {

        }
    }
    av_packet_free(&packet);
    LOG_INFO << PLAYTAG << "FrameReader end";
}
