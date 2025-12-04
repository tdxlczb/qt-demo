#include "media_decoder.h"
#include "media_utils.h"
#include "log.h"

namespace mp {

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
    LOG_WARN << "HwPixFmt not found " << pDecoder->GetHwPixFmt();
    pDecoder->QuitHwDecode();
    return AV_PIX_FMT_NONE;
}

VideoDecoder::VideoDecoder()
{
}

VideoDecoder::~VideoDecoder()
{
}

const AVPixelFormat& VideoDecoder::GetHwPixFmt() const
{
    return m_hwPixFmt;
}

void VideoDecoder::QuitHwDecode()
{
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
}

bool VideoDecoder::OpenDecoder(AVCodecID id, const std::string& hwdevice, AVCodecParameters* codecpar)
{
    // 获取视频解码器
    const AVCodec* videoCodec = avcodec_find_decoder(id);
    if (!videoCodec) {
        LOG_ERROR << "can not find decoder:" << id;
        return false;
    }
    // 创建解码器上下文
    m_videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!m_videoCodecContext) {
        LOG_ERROR << "create decoder failed:" << id;
        return false;
    }
    int ret = 0;
    if (codecpar) {
        ret = avcodec_parameters_to_context(m_videoCodecContext, codecpar);
        if (AVERROR(ret)) {
            LOG_ERROR << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            CloseDecoder();
            return false;
        }
    }

    //m_videoCodecContext->err_recognition |= AV_EF_EXPLODE;//设置解码器的错误恢复标志，可以跳过错误的帧，从而减少花屏现象
    m_videoCodecContext->opaque = this; //用于回调里获取的指针
    m_videoCodecContext->thread_count = 0;

    if (!hwdevice.empty()) {
        m_hwDeviceType = av_hwdevice_find_type_by_name(hwdevice.c_str());
        if (m_hwDeviceType == AV_HWDEVICE_TYPE_NONE) {
            LOG_ERROR << "device type is not supported:" << hwdevice;
            return false;
        }
        //查找硬解码器
        for (int i = 0;; ++i) {
            const AVCodecHWConfig* codecHWConfig = avcodec_get_hw_config(videoCodec, i);
            if (!codecHWConfig) {
                //没有硬解码器了
                LOG_WARN << string_format("decoder %s is not support device type:%s", videoCodec->name, av_hwdevice_get_type_name(m_hwDeviceType));
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
                    LOG_WARN << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                }
            }
            m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_dxva2_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
        }
        else if (m_hwDeviceType == AV_HWDEVICE_TYPE_D3D11VA) {
            if (!g_d3d11_device) {
                ret = av_hwdevice_ctx_create(&g_d3d11_device, m_hwDeviceType, nullptr, nullptr, 0);
                if (ret != 0) {
                    LOG_WARN << "av_hwdevice_ctx_create failed," << ret << ":" << av_error_string(ret);
                    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;
                }
            }
            m_videoCodecContext->hw_device_ctx = av_buffer_ref(g_d3d11_device);//这里使用av_buffer_ref，就能安全释放m_videoCodecContext，避免复用的device被删除
        }
        m_videoCodecContext->get_format = GetHwFormat;
    }

    ret = avcodec_open2(m_videoCodecContext, videoCodec, nullptr);
    if (ret != 0) {
        LOG_ERROR << "video avcodec_open2 failed," << ret << ":" << av_error_string(ret);
        CloseDecoder();
        return false;
    }
    return true;
}

void VideoDecoder::CloseDecoder()
{
    m_callback = nullptr;
    if (m_videoCodecContext)
    {
        if (avcodec_is_open(m_videoCodecContext))
            avcodec_close(m_videoCodecContext);
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
    }
    m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    m_inputIndex = 0;
    m_outputIndex = 0;
}

bool VideoDecoder::IsOpen()
{
    return m_videoCodecContext != nullptr;
}

void VideoDecoder::SetOnDecodeFrame(OnDecodeFrame callback)
{
    m_callback = callback;
}

void VideoDecoder::SendPacket(AVPacket* packet)
{
    if (!m_videoCodecContext)
        return;

    m_inputIndex++;
    bool isKey = packet->flags & AV_PKT_FLAG_KEY;
    //LOG_DEBUG << "packet size:" << packet->size << ", pts:" << packet->pts << ", isKey:" << isKey << ", packetIndex:" << m_inputIndex;
    int ret = avcodec_send_packet(m_videoCodecContext, packet);
    if (ret < 0)
    {//错误处理
        LOG_ERROR << "avcodec_send_packet err," << ret << ":" << av_error_string(ret) << ", pts:" << packet->pts << ", isKey:" << isKey << ", packetIndex:" << m_inputIndex;
        return;
        // 处理返回值
        if (ret == AVERROR(EAGAIN)) {
            // 需要先接收帧
            return;
        }
        else if (ret == AVERROR_EOF) {
            // 解码器已冲刷，退出
            return;
        }
        else if (ret == AVERROR_INVALIDDATA) {
            // 警告性错误：记录日志但继续
            //LOG_WARN << "Invalid packet: pts=" << packet->pts << " size=" << packet->size;
            // 不要break，继续下一个包
        }
        else {
            // 其他错误
        }

    }

    while (true)
    {
        AVFrame* frame = av_frame_alloc();
        ret = avcodec_receive_frame(m_videoCodecContext, frame);
        if (ret < 0)
        {//错误处理
            //LOG_ERROR << "avcodec_receive_frame err," << ret << ":" << av_error_string(ret);
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
                    LOG_ERROR << "av_hwframe_transfer_data err," << ret << ":" << av_error_string(ret);
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
}

void VideoDecoder::FlushBuffers()
{
    avcodec_flush_buffers(m_videoCodecContext);
}



AudioDecoder::AudioDecoder()
{
}

AudioDecoder::~AudioDecoder()
{
}

bool AudioDecoder::OpenDecoder(AVCodecID id, AVCodecParameters* codecpar)
{
    // 获取视频解码器
    const AVCodec* audioCodec = avcodec_find_decoder(id);
    if (!audioCodec) {
        LOG_ERROR << "can not find decoder:" << id;
        return false;
    }
    // 创建解码器上下文
    m_audioCodecContext = avcodec_alloc_context3(audioCodec);
    if (!m_audioCodecContext) {
        LOG_ERROR << "create decoder failed:" << id;
        return false;
    }
    int ret = 0;
    if (codecpar) {
        ret = avcodec_parameters_to_context(m_audioCodecContext, codecpar);
        if (AVERROR(ret)) {
            LOG_ERROR << "avcodec_parameters_to_context failed," << ret << ":" << av_error_string(ret);
            CloseDecoder();
            return false;
        }
    }

    ret = avcodec_open2(m_audioCodecContext, audioCodec, nullptr);
    if (ret != 0) {
        LOG_ERROR << "audio avcodec_open2 failed," << ret << ":" << av_error_string(ret);
        CloseDecoder();
        return false;
    }
    return true;
}

void AudioDecoder::CloseDecoder()
{
    m_callback = nullptr;
    if (m_audioCodecContext)
    {
        if (avcodec_is_open(m_audioCodecContext))
            avcodec_close(m_audioCodecContext);
        avcodec_free_context(&m_audioCodecContext);
        m_audioCodecContext = nullptr;
    }
    m_inputIndex = 0;
    m_outputIndex = 0;
}

bool AudioDecoder::IsOpen()
{
    return m_audioCodecContext != nullptr;
}

void AudioDecoder::SetOnDecodeFrame(OnDecodeFrame callback)
{
    m_callback = callback;
}

void AudioDecoder::SendPacket(AVPacket* packet)
{
    if (!m_audioCodecContext)
        return;
    m_inputIndex++;
    int ret = avcodec_send_packet(m_audioCodecContext, packet);
    if (ret < 0)
    {//错误处理
        LOG_ERROR << "avcodec_send_packet err," << ret << ":" << av_error_string(ret);
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
        m_outputIndex++;
        if (m_callback) {
            m_callback(frame);
        }
        av_frame_free(&frame);
    }
}

void AudioDecoder::FlushBuffers()
{
    avcodec_flush_buffers(m_audioCodecContext);
}

} // namespace mp