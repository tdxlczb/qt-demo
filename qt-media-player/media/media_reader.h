#ifndef MEDIA_READER_H
#define MEDIA_READER_H

#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "media_define.h"
#include <opencv2/opencv.hpp>
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


/*
* 为实现音画同步效果
* 视频采用回调方式渲染
* 音频采用主动获取方式渲染
*/

class FrameQueue
{
public:
    FrameQueue(int16_t maxQueueSize = 5);
    ~FrameQueue();

    void Push(AVFrame* frame);
    AVFrame* PopFront();
    void Clear();
private:
    std::queue<AVFrame*> m_frameQueue;
    const int16_t m_maxQueueSize = 0;
    std::mutex m_frameQueueMutex;
    std::condition_variable m_queueCV;
};

class VideoDisplay;
class AudioDisplay;
class DynamicJitterBuffer;
class MediaReader
{
public:
    MediaReader();
    ~MediaReader();

    bool Init(const MediaParameter& param);
    void UnInit();
    bool Start();
    bool Stop();
    void UpdateDisplaySize(int width, int height);
    void SetPlayEvent(PlayEvent* playEvent);
    size_t GetAudioFrame(uint8_t* buffer, size_t len);

    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //退出硬解码（在非本类代码内执行时可用）
    void QuitHwDecode();
private:
    void ReadThread();
    void VideoDecode(AVPacket* packet);
    void AudioDecode(AVPacket* packet);

    void VideoThread();
    void AudioThread();
    /*
    * 做音画同步时，直接使用av_sleep做延迟会由于精度问题无法准确同步
    * 可以使用有更高精度的sleep去做延迟，或者创建新线程，使用while循环比较时间，代替sleep
    */
    void DisplayThread();

    void DisplayVideo(const VideoFrame& frame);
    void FrameReaderSync();

    //判断是否是灰帧
    bool IsGrayFrame(AVFrame* frame);
private:
    bool m_isInit = false;
    MediaParameter m_param;
    PlayEvent* m_playEvent = nullptr;
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

    VideoDisplay* m_pGrayFrameDisplay = nullptr;
    VideoDisplay* m_pVideoDisplay = nullptr;
    AudioDisplay* m_pAudioDisplay = nullptr;

    double m_lastFrameRenderTime = 0; //上一帧的播放时刻av_gettime_relative，不能使用系统时间，系统时间是可以任意时刻更改的
    double m_lastDelayDelta = 0; //上一帧delay和实际delay的间隔时间，由于av_sleep有精度问题，需要记录这个差距，下次进行调整
    int64_t m_lastVideoFramePts = 0;  
    double m_masterClock = 0.0;       //主时钟
    double m_syncThreshold = 0.1;     //同步阈值
    double m_speed = 1.0;             //倍速播放

    FrameQueue m_videoFrameQueue{ 10 };
    FrameQueue m_audioFrameQueue{ 10 };
    DynamicJitterBuffer* m_audioBuffer = nullptr;
};

#endif // MEDIA_READER_H
