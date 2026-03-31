#include "media_decoder.h"
#include "media_utils.h"
#include "media_log.h"

namespace mp {


MediaDecoder::MediaDecoder(const std::string& context)
    :MediaContext(context)
{}

MediaDecoder::~MediaDecoder()
{}

bool MediaDecoder::IsOpen()
{
    return m_codecContext != nullptr;
}

void MediaDecoder::SetOnDecodeFrame(OnDecodeFrame callback)
{
    m_callback = callback;
}

void MediaDecoder::FlushBuffers()
{
    if (m_codecContext) {
        avcodec_flush_buffers(m_codecContext);
    }
}


//复用同一个AVHWDeviceContext能减少大量cpu和gpu
static AVBufferRef* g_d3d11_device = nullptr;
static AVBufferRef* g_dxva2_device = nullptr;

static AVPixelFormat GetHwFormat(AVCodecContext* pCodecContext, const enum AVPixelFormat* pPixFmts)
{
    VideoDecoder* pDecoder = (VideoDecoder*)pCodecContext->opaque;
    if (!pDecoder)
        return AV_PIX_FMT_NONE;

    const enum AVPixelFormat* pPixFmt;
    //遍历给定的AVPixelFormat数组中存不存在AVCodecContext支持的硬解码格式
    for (pPixFmt = pPixFmts; *pPixFmt != AV_PIX_FMT_NONE; ++pPixFmt)
    {
        if (*pPixFmt == pDecoder->GetHwPixFmt())
        {
            return *pPixFmt;
        }
    }
    LOG_WARN << "[" << pDecoder->GetContext() << "] " << "HwPixFmt not found " << pDecoder->GetHwPixFmt();
    pDecoder->TryQuitHwDecode();
    return AV_PIX_FMT_NONE;
}

VideoDecoder::VideoDecoder(const std::string& context)
    : MediaDecoder(context)
{}

VideoDecoder::~VideoDecoder()
{
    CloseDecoder();
}

const AVPixelFormat& VideoDecoder::GetHwPixFmt() const
{
    return m_hwPixFmt;
}

void VideoDecoder::TryQuitHwDecode()
{
    m_tryQuitHwCount = 1;
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
}

bool VideoDecoder::QuitHwDecode()
{
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
    LOG_INFO_T << "QuitHwDecode";
    if (m_codecContext) {
        avcodec_close(m_codecContext);
    }
    if (m_codecContext->hw_device_ctx) {
        av_buffer_unref(&m_codecContext->hw_device_ctx);
        m_codecContext->hw_device_ctx = nullptr;
    }
    m_codecContext->opaque = nullptr;
    m_codecContext->get_format = m_getFormat;

    int ret = avcodec_open2(m_codecContext, m_pCodec, NULL);
    if (ret != 0) {
        LOG_ERROR_T << "video avcodec_open2 failed," << ret << ":" << av_error_string(ret);
        CloseDecoder();
        m_tryQuitHwCount++;
        return false;
    }
    m_tryQuitHwCount = 0;
    return true;
}

bool VideoDecoder::OpenDecoder(const PlayOptions& options, AVCodecParameters* codecpar)
{
    AVCodecID id = codecpar->codec_id;
    // 获取视频解码器
    const AVCodec* videoCodec = avcodec_find_decoder(id);
    if (!videoCodec) {
        LOG_ERROR_T << "can not find decoder:" << id;
        return false;
    }
    // 创建解码器上下文
    m_codecContext = avcodec_alloc_context3(videoCodec);
    if (!m_codecContext) {
        LOG_ERROR_T << "create decoder failed:" << id;
        return false;
    }
    int ret = 0;
    if (codecpar) {
        ret = avcodec_parameters_to_context(m_codecContext, codecpar);
        if (AVERROR(ret)) {
            LOG_ERROR_T << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            CloseDecoder();
            return false;
        }
    }

    //m_codecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
    m_codecContext->opaque = this; //用于回调里获取的指针
    m_codecContext->thread_count = 0; // 这里开启多少线程，解码器就会缓存多少个packet，如果缓存packet太多，会导致解码延迟太高

    if (!options.hwdevice.empty()) {
        m_hwDeviceType = av_hwdevice_find_type_by_name(options.hwdevice.c_str());
        if (m_hwDeviceType == AV_HWDEVICE_TYPE_NONE) {
            LOG_ERROR_T << "device type is not supported:" << options.hwdevice;
            return false;
        }
        //查找硬解码器
        for (int i = 0;; ++i) {
            const AVCodecHWConfig* codecHWConfig = avcodec_get_hw_config(videoCodec, i);
            if (!codecHWConfig) {
                //没有硬解码器了
                LOG_WARN_T << string_format("decoder %s is not support device type:%s", videoCodec->name, av_hwdevice_get_type_name(m_hwDeviceType));
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
                if (ret != 0) {
                    LOG_WARN_T << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                }
            }
            m_codecContext->hw_device_ctx = av_buffer_ref(g_dxva2_device);//这里使用av_buffer_ref，就能安全释放m_codecContext，避免复用的device被删除
        } else if (m_hwDeviceType == AV_HWDEVICE_TYPE_D3D11VA) {
            if (!g_d3d11_device) {
                ret = av_hwdevice_ctx_create(&g_d3d11_device, m_hwDeviceType, nullptr, nullptr, 0);
                if (ret != 0) {
                    LOG_WARN_T << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                }
            }
            m_codecContext->hw_device_ctx = av_buffer_ref(g_d3d11_device);//这里使用av_buffer_ref，就能安全释放m_codecContext，避免复用的device被删除
        }
        m_getFormat = m_codecContext->get_format; //保存原来的方法，退出硬解码时使用
        m_codecContext->get_format = GetHwFormat;
    }
    m_pCodec = videoCodec; //解码器，退出硬解码时使用
    ret = avcodec_open2(m_codecContext, videoCodec, nullptr);
    if (ret != 0) {
        LOG_ERROR_T << "video avcodec_open2 failed," << ret << ":" << av_error_string(ret);
        CloseDecoder();
        return false;
    }

    // 硬解码时，例如amd设备支持的dxva2解码器最大分辨率比视频分辨率小，可能会出现初始avcodec_open2打开硬解码时，get_format查找硬解码格式时都能成功
    // 但是在avcodec_send_packet中触发get_format查找不到硬解码，因此需要在触发这个问题后重新初始软解码器
    if (m_tryQuitHwCount > 0) {
        if (!QuitHwDecode()) {
            CloseDecoder();
            return false;
        }
    }

    return true;
}

void VideoDecoder::CloseDecoder()
{
    m_callback = nullptr;
    if (m_codecContext)
    {
        // avcodec_close不再使用，avcodec_free_context内部包含avcodec_close和avcodec_is_open判断
        //if (avcodec_is_open(m_codecContext))
        //    avcodec_close(m_codecContext);
        avcodec_free_context(&m_codecContext);
        m_codecContext = nullptr;
    }
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    m_inputIndex = 0;
    m_outputIndex = 0;
}

bool VideoDecoder::SendPacket(AVPacket* packet)
{
    if (!m_codecContext)
        return false;

    m_inputIndex++;
    bool isKey = packet->flags & AV_PKT_FLAG_KEY;
    //LOG_DEBUG << "packet size:" << packet->size << ", pts:" << packet->pts << ", isKey:" << isKey << ", packetIndex:" << m_inputIndex;
    int ret = avcodec_send_packet(m_codecContext, packet);
    if (ret < 0)
    {//错误处理
        LOG_ERROR_T << "avcodec_send_packet err," << ret << ":" << av_error_string(ret) << ", pts:" << packet->pts << ", isKey:" << isKey << ", packetIndex:" << m_inputIndex;
        // 处理返回值
        if (ret == AVERROR(EAGAIN)) {
            // 解码器需要更多数据，需要先接收帧
            return true;
        } else if (ret == AVERROR_EOF) {
            // 解码器已冲刷，退出
            return true;
        } else if (ret == AVERROR_INVALIDDATA) {
            // 输入数据有问题，可能数据不完整:Invalid data found when processing input
            // 警告性错误：记录日志但继续
            // LOG_WARN << "Invalid packet: pts=" << packet->pts << " size=" << packet->size;
            if (m_tryQuitHwCount > 0 && m_tryQuitHwCount <= 10) {//如果是因为硬解被退出了导致的这个报错，则终止解帧并传出错误，尝试次数太多则放弃
                LOG_ERROR_T << "hwdecode error, try quit hwdecode:" << m_tryQuitHwCount;
                QuitHwDecode();
                return false;
            }
            return false;
        } else {
            // 其他错误
        }
        return false;
    }

    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_codecContext, frame);
        if (ret < 0)
        {//错误处理
            //LOG_ERROR_T << "avcodec_receive_frame err," << ret << ":" << av_error_string(ret);
            av_frame_free(&frame);
            break;
        }
        m_outputIndex++;
        //if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
        //    av_frame_free(&frame);
        //    continue;
        //}
        if (frame->format == m_hwPixFmt) {
            // 从GPU内存下载到cpu内存
            // av_hwframe_map效率比av_hwframe_transfer_data高，耗时减少约1/3，但是性能好像并没有优化多少，cpu降低了，但是gpu变高了
            // sw_frame->format 现在是 AV_PIX_FMT_DRM_PRIME / CUDA / D3D11 等
            AVFrame* hwFrame = av_frame_alloc();
            ret = av_hwframe_map(hwFrame, frame, AV_HWFRAME_MAP_READ);
            if (AVERROR(ret)) {
                ret = av_hwframe_transfer_data(hwFrame, frame, 0);
                if (AVERROR(ret)) {
                    LOG_ERROR_T << "av_hwframe_transfer_data err," << ret << ":" << av_error_string(ret);
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
        if (m_callback) {
            m_callback(frame);
        }
        av_frame_free(&frame);
    }
    return true;
}


AudioDecoder::AudioDecoder(const std::string& context)
    : MediaDecoder(context)
{}

AudioDecoder::~AudioDecoder()
{
    CloseDecoder();
}

bool AudioDecoder::OpenDecoder(const PlayOptions& options, AVCodecParameters* codecpar)
{
    AVCodecID id = codecpar->codec_id;
    // 获取视频解码器
    const AVCodec* audioCodec = avcodec_find_decoder(id);
    if (!audioCodec) {
        LOG_ERROR_T << "can not find decoder:" << id;
        return false;
    }
    // 创建解码器上下文
    m_codecContext = avcodec_alloc_context3(audioCodec);
    if (!m_codecContext) {
        LOG_ERROR_T << "create decoder failed:" << id;
        return false;
    }
    int ret = 0;
    if (codecpar) {
        ret = avcodec_parameters_to_context(m_codecContext, codecpar);
        if (AVERROR(ret)) {
            LOG_ERROR_T << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            CloseDecoder();
            return false;
        }
    }
    m_codecContext->thread_count = 0;
    ret = avcodec_open2(m_codecContext, audioCodec, nullptr);
    if (ret != 0) {
        LOG_ERROR_T << "audio avcodec_open2 failed," << ret << ":" << av_error_string(ret);
        CloseDecoder();
        return false;
    }
    return true;
}

void AudioDecoder::CloseDecoder()
{
    m_callback = nullptr;
    if (m_codecContext)
    {
        // avcodec_close不再使用，avcodec_free_context内部包含avcodec_close和avcodec_is_open判断
        //if (avcodec_is_open(m_codecContext))
        //    avcodec_close(m_codecContext);
        avcodec_free_context(&m_codecContext);
        m_codecContext = nullptr;
    }
    m_inputIndex = 0;
    m_outputIndex = 0;
}

bool AudioDecoder::SendPacket(AVPacket* packet)
{
    if (!m_codecContext)
        return false;
    m_inputIndex++;
    int ret = avcodec_send_packet(m_codecContext, packet);
    if (ret < 0)
    {//错误处理
        LOG_ERROR_T << "avcodec_send_packet err," << ret << ":" << av_error_string(ret);
        return false;
    }
    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_codecContext, frame);
        if (ret < 0)
        {//错误处理
            av_frame_free(&frame);
            break;
        }
        m_outputIndex++;
        if (m_callback) {
            m_callback(frame);
        }
        av_frame_free(&frame);
    }

    return true;
}


} // namespace mp