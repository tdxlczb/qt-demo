#ifndef MEDIA_DECODER_H
#define MEDIA_DECODER_H

#include <string>
#include <functional>
extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace mp {

using OnDecodeFrame = std::function<void(AVFrame*)>;
typedef AVPixelFormat(*FunGetFormat)(struct AVCodecContext* s, const enum AVPixelFormat* fmt); //解码的方法

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    bool OpenDecoder(AVCodecID id, const std::string& hwdevice = "", AVCodecParameters* codecpar = nullptr);
    void CloseDecoder();
    bool IsOpen();

    void SetOnDecodeFrame(OnDecodeFrame callback);
    bool SendPacket(AVPacket* packet);
    void FlushBuffers();
    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //准备退出硬解码
    void TryQuitHwDecode();
    //退出硬解码
    bool QuitHwDecode();
private:
    AVCodecContext* m_videoCodecContext = nullptr;
    const AVCodec* m_pCodec = nullptr;        //解码器
    enum AVHWDeviceType m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    enum AVPixelFormat m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    OnDecodeFrame m_callback;
    int64_t m_inputIndex = 0;
    int64_t m_outputIndex = 0;
    FunGetFormat m_getFormat = NULL;
    int m_tryQuitHwCount = 0;            //尝试退出硬解码的次数
};


class AudioDecoder
{
public:
    AudioDecoder();
    ~AudioDecoder();

    bool OpenDecoder(AVCodecID id, AVCodecParameters* codecpar = nullptr);
    void CloseDecoder();
    bool IsOpen();

    void SetOnDecodeFrame(OnDecodeFrame callback);
    void SendPacket(AVPacket* packet);
    void FlushBuffers();
private:
    AVCodecContext* m_audioCodecContext = nullptr;
    OnDecodeFrame m_callback;
    int64_t m_inputIndex = 0;
    int64_t m_outputIndex = 0;
};

} // namespace mp


#endif // MEDIA_DECODER_H
