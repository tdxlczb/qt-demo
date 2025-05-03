#ifndef MEDIAREADER_H
#define MEDIAREADER_H

#include <thread>
#include <atomic>
#include <condition_variable>
#include "media_define.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

class MediaReader
{

public:
    MediaReader();
    ~MediaReader();

    bool Init(const MediaParameter& pParam);
    void UnInit();
    bool Start();
    bool Stop();

private:
    void FrameReader();
    void VideoDecode(AVPacket* packet);
    void AudioDecode(AVPacket* packet);

    void VideoProcess();
    void AudioProcess();

private:
    bool m_isInit = false;
    MediaParameter m_param;
    std::thread m_thFrameReader;
    std::atomic_bool m_bFrameReaderRunning {false}; //atomic在gcc编译器中不可使用=进行初始化
    std::thread m_thVideoProcess;
    std::atomic_bool m_bVideoProcessRunning {false};
    std::thread m_thAudioProcess;
    std::atomic_bool m_bAudioProcessRunning {false};

    AVFormatContext* m_formatContext = nullptr;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    AVCodecContext* m_videoCodecContext = nullptr;
    AVCodecContext* m_audioCodecContext = nullptr;
    SwsContext* m_videoSwsContext = nullptr;//视频重采样
    bool m_videoNeedConvert = false;
    SwsContext* m_audioSwsContext = nullptr;//音频重采样
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
    std::condition_variable m_audioFrameQueueCV;
};

#endif // MEDIAREADER_H
