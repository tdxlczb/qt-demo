#ifndef MEDIAREADER_H
#define MEDIAREADER_H

#include <thread>
#include <atomic>
#include <condition_variable>
#include "media_define.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

class MediaReader
{

public:
    MediaReader();
    ~MediaReader();

    bool Init(const MediaParameter& pParam);
    void UnInit();
    bool Start();
    bool Stop();

    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //退出硬解码（在非本类代码内执行时可用）
    void QuitHwDecode();
private:
    void FrameReader();
    void VideoDecode(AVPacket* packet);
    void AudioDecode(AVPacket* packet);

    void VideoProcess();
    void AudioProcess();

    void FrameReaderSync();
private:
    bool m_isInit = false;
    MediaParameter m_param;
    std::thread m_thFrameReader;
    std::atomic_bool m_bFrameReaderRunning{ false }; //atomic在gcc编译器中不可使用=进行初始化
    std::thread m_thVideoProcess;
    std::atomic_bool m_bVideoProcessRunning{ false };
    std::thread m_thAudioProcess;
    std::atomic_bool m_bAudioProcessRunning{ false };
    std::atomic_bool m_bStreamOver{ false };

    AVFormatContext* m_formatContext = nullptr;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    AVCodecContext* m_videoCodecContext = nullptr;
    AVCodecContext* m_audioCodecContext = nullptr;
    enum AVHWDeviceType m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    enum AVPixelFormat m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    SwsContext* m_videoSwsContext = nullptr;//视频重采样
    bool m_videoNeedConvert = false;
    SwrContext* m_audioSwrContext = nullptr;//音频重采样
    bool m_audioNeedConvert = false;
    int64_t m_lastVideoFramePts = 0;

    const int16_t kMaxVideoFrameQueueSize = 5;
    std::queue<AVFrame*> m_videoFrameQueue;
    std::mutex m_videoFrameQueueMutex;
    std::condition_variable m_videoFrameQueueNotFullCV;//队列未满的条件变量
    std::condition_variable m_videoFrameQueueNotEmptyCV;//队列非空的条件变量

    const int16_t kMaxAudioFrameQueueSize = 5;
    std::queue<AVFrame*> m_audioFrameQueue;
    std::mutex m_audioFrameQueueMutex;
    std::condition_variable m_audioFrameQueueNotFullCV;//队列未满的条件变量
    std::condition_variable m_audioFrameQueueNotEmptyCV;//队列非空的条件变量
};

#endif // MEDIAREADER_H
