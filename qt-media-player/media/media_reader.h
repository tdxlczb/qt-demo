#ifndef MEDIA_READER_H
#define MEDIA_READER_H

#include <thread>
#include <atomic>
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

#include "media_define.h"
#include "media_play_event.h"
#include "media_queue.h"

/*
* 为实现音画同步效果
* 视频采用回调方式渲染
* 音频采用主动获取方式渲染
* 
* 解码和取包不要在统一个线程，避免性能不足的时候可能会影响取包，导致网络包延迟累积
*/

class MediaReader
{
public:
    MediaReader(int index);
    ~MediaReader();

    void Play(const std::string& url, const PlayOptions& options);
    void Stop();
    void SetPlayEvent(PlayEvent* playEvent);

    //获取当前硬解码的图像格式(仅硬解码时有效)
    const AVPixelFormat& GetHwPixFmt() const;
    //退出硬解码（在非本类代码内执行时可用）
    void QuitHwDecode();
private:
    void ReadThread();
    void VideoThread();
    void AudioThread();
    void VideoDecode(AVPacket* packet);
    void AudioDecode(AVPacket* packet);

    /*
    * 做音画同步时，直接使用av_sleep做延迟会由于精度问题无法准确同步
    * 可以使用有更高精度的sleep去做延迟，或者创建新线程，使用while循环比较时间，代替sleep
    */
    void DisplayThread();
    void DisplayVideo(AVFrame* frame);

    bool StreamOpen();
    void StreamClose();

    void FrameReaderSync();

private:
    int m_playIndex = 0;
    std::string m_url;
    PlayOptions m_options;
    PlayEvent* m_playEvent = nullptr;
    std::atomic_bool m_isThreadRun{ false }; //atomic在gcc编译器中不可使用=进行初始化
    std::thread m_thReader;
    std::thread m_thVideoDecoder;
    std::thread m_thAudioDecoder;
    std::thread m_thVideoDisplay;
    std::atomic_bool m_isStreamOver{ false };

    AVFormatContext* m_formatContext = nullptr;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    AVCodecContext* m_videoCodecContext = nullptr;
    AVCodecContext* m_audioCodecContext = nullptr;
    enum AVHWDeviceType m_hwDeviceType = AV_HWDEVICE_TYPE_NONE;//硬解码类型
    enum AVPixelFormat m_hwPixFmt = AV_PIX_FMT_NONE;//硬解码的格式
    PacketQueue m_videoPacketQueue;
    PacketQueue m_audioPacketQueue;

    int64_t m_videoFrameIndex = 0;
    int64_t m_audioFrameIndex = 0;
    int64_t m_iLastCountTime = 0;
    double m_lastFrameRenderTime = 0; //上一帧的播放时刻av_gettime_relative，不能使用系统时间，系统时间是可以任意时刻更改的
    double m_lastDelayDelta = 0; //上一帧delay和实际delay的间隔时间，由于av_sleep有精度问题，需要记录这个差距，下次进行调整
    int64_t m_lastVideoFramePts = 0;  
    double m_masterClock = 0.0;       //主时钟
    double m_syncThreshold = 0.1;     //同步阈值
    double m_speed = 1.0;             //倍速播放
};

#endif // MEDIA_READER_H
