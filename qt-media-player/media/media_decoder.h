#ifndef MEDIA_DECODER_H
#define MEDIA_DECODER_H

#include <string>
#include <functional>
extern "C"
{
#include <libavcodec/avcodec.h>
}

//namespace media {

using OnDecodeFrame = std::function<void(AVFrame*)>;

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    bool OpenDecoder(AVCodecID id, const std::string& hwdevice = "", AVCodecParameters* codecpar = nullptr);
    void CloseDecoder();

    void SetOnDecodeFrame(OnDecodeFrame callback);
    void SendPacket(AVPacket* packet);
    void FlushBuffers();
    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //退出硬解码（在非本类代码内执行时可用）
    void QuitHwDecode();
private:
    AVCodecContext* m_videoCodecContext = nullptr;
    enum AVHWDeviceType m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    enum AVPixelFormat m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    OnDecodeFrame m_callback;
    int64_t m_inputIndex = 0;
    int64_t m_outputIndex = 0;
};


class AudioDecoder
{
public:
    AudioDecoder();
    ~AudioDecoder();

    bool OpenDecoder(AVCodecID id, AVCodecParameters* codecpar = nullptr);
    void CloseDecoder();

    void SetOnDecodeFrame(OnDecodeFrame callback);
    void SendPacket(AVPacket* packet);
    void FlushBuffers();
private:
    AVCodecContext* m_audioCodecContext = nullptr;
    OnDecodeFrame m_callback;
};

//} // namespace media


#endif // MEDIA_DECODER_H
