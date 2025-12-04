#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <thread>
#include <atomic>

#include "media_define.h"
#include "media_play_event.h"
#include "media_queue.h"
#include "media_clock.h"

extern "C"
{
#include <libavcodec/codec_id.h>
}

#define PLAYTAG  m_tag + " " 

struct AVCodecParameters;

namespace mp {
/*
* 为实现音画同步效果
* 视频采用回调方式渲染
* 音频采用主动获取方式渲染
*
* 解码和取包不要在统一个线程，避免性能不足的时候可能会影响取包，导致网络包延迟累积
*/
class AudioSpeedFilter;
class VideoDecoder;
class AudioDecoder;
class MediaPlayer
{
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    std::string GetId() const;
    std::string GetTag() const;
    std::string GetPlayUrl() const;
    void SetTag(const std::string& tag);
    void SetPlayEvent(PlayEvent* playEvent);

    virtual void Play(const std::string& url, const PlayOptions& options);
    virtual void Stop();
    virtual void Pause();
    virtual void Resume();
    virtual void Speed(double speed);
    virtual void Seek(int64_t seconds);//跳转从当前时间计算的一段时间
    virtual void SeekTo(int64_t seconds);//跳转到从开始时间计算的一段时间
    virtual int64_t GetDuration();

protected:
    virtual bool StreamOpen();
    virtual void StreamClose();
    virtual void StreamDemux();
    bool CreateVideoDecoder(AVCodecID id, AVCodecParameters* codecpar);
    bool CreateAudioDecoder(AVCodecID id, AVCodecParameters* codecpar);
    void CloseDecoder();

    void DemuxThread();
    void VideoThread();
    void AudioThread();
    void OnVideoFrame(AVFrame* frame);
    void OnAudioFrame(AVFrame* frame);

    /*
    * 做音画同步时，直接使用av_sleep做延迟会由于精度问题无法准确同步
    * 可以使用有更高精度的sleep去做延迟，或者创建新线程，使用while循环比较时间，代替sleep
    */
    void DisplayThread();
    void DisplayVideo(AVFrame* frame);
    void DisplayAudio(AVFrame* frame);

protected:
    const std::string m_uid; //类的唯一id
    std::string m_tag = "tag"; //标记，主要用于绑定外部播放窗口，日志区分
    std::string m_url;
    PlayOptions m_options;
    PlayEvent* m_playEvent = nullptr;
    std::atomic_bool m_isThreadRun{ false }; //atomic在gcc编译器中不可使用=进行初始化
    std::thread m_thDemuxer;
    std::thread m_thVideoDecoder;
    std::thread m_thAudioDecoder;
    std::thread m_thVideoDisplay;
    std::atomic_bool m_isStreamOver{ false };

    bool m_isMediaFile = false;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;

    PacketQueue m_videoPacketQueue;
    PacketQueue m_audioPacketQueue;

    bool m_isSyncDemux = false;    //同步解复用
    bool m_isAsyncDisplay = false; //异步播放
    FrameQueue m_videoFrameQueue;

    int64_t m_videoPacketIndex = 0;
    int64_t m_audioPacketIndex = 0;
    int64_t m_videoPacketSize = 0; //用于计算码率
    int64_t m_audioPacketSize = 0; //用于计算码率
    int64_t m_lastCountPacketSizeTime = 0;
    double m_lastVideoPts = 0.0;

    int64_t m_videoFrameIndex = 0;
    int64_t m_audioFrameIndex = 0;
    int64_t m_iLastCountTime = 0;

    MediaClock m_audioClock;
    MediaClock m_videoClock;
    MediaClock m_extClock;  // 外部时钟
    double m_videoTimebae = 0.0;
    double m_audioTimebae = 0.0;

    double m_speed = 1.0;             //倍速播放
    AudioSpeedFilter* m_pAudioSpeedFilter = nullptr;
    std::atomic_bool m_seekReq{ false };
    int64_t m_seekPos = 0;
    std::atomic_bool m_pauseReq{ false };
    std::atomic_bool m_isPaused{ false };

    std::unique_ptr<VideoDecoder> m_pVideoDecoder;
    std::unique_ptr<AudioDecoder> m_pAudioDecoder;
};

} // namespace mp

#endif // MEDIA_PLAYER_H
