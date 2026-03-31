#ifndef MEDIA_DECODER_H
#define MEDIA_DECODER_H

#include <string>
#include <functional>
extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "media_define.h"
#include "media_context.h"

namespace mp {

using OnDecodeFrame = std::function<void(AVFrame*)>;
typedef AVPixelFormat(*FunGetFormat)(struct AVCodecContext* s, const enum AVPixelFormat* fmt); //解码的方法

class MediaDecoder : public MediaContext
{
public:
    MediaDecoder(const std::string& context);
    ~MediaDecoder();

    virtual bool OpenDecoder(const PlayOptions& options, AVCodecParameters* codecpar) = 0;
    virtual void CloseDecoder() = 0;
    virtual bool SendPacket(AVPacket* packet) = 0;

    virtual bool IsOpen();
    virtual void SetOnDecodeFrame(OnDecodeFrame callback);
    virtual void FlushBuffers();

protected:
    AVCodecContext* m_codecContext = nullptr;
    OnDecodeFrame m_callback;
};

class VideoDecoder : public MediaDecoder
{
public:
    VideoDecoder(const std::string& context);
    ~VideoDecoder();

    bool OpenDecoder(const PlayOptions& options, AVCodecParameters* codecpar) override;
    void CloseDecoder() override;
    bool SendPacket(AVPacket* packet) override;


    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //准备退出硬解码
    void TryQuitHwDecode();
    //退出硬解码
    bool QuitHwDecode();

private:
    const AVCodec* m_pCodec = nullptr;        //解码器
    enum AVHWDeviceType m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    enum AVPixelFormat m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    FunGetFormat m_getFormat = NULL;
    int m_tryQuitHwCount = 0;            //尝试退出硬解码的次数
    int64_t m_inputIndex = 0;
    int64_t m_outputIndex = 0;
};


class AudioDecoder : public MediaDecoder
{
public:
    AudioDecoder(const std::string& context);
    ~AudioDecoder();

    bool OpenDecoder(const PlayOptions& options, AVCodecParameters* codecpar) override;
    void CloseDecoder() override;
    bool SendPacket(AVPacket* packet) override;

private:
    int64_t m_inputIndex = 0;
    int64_t m_outputIndex = 0;
};

} // namespace mp


#endif // MEDIA_DECODER_H
