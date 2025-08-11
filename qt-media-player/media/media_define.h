#ifndef MEDIA_DEFINE_H
#define MEDIA_DEFINE_H

#include <string>
#include <functional>


enum class PlayErrorCode
{
    kNoError = 0,
    kCreateConverterFailed,  //视频/音频格式重采样器创建失败
    kNoConverter,            //没有视频/音频格式重采样器
    kConverteFailed,         //视频/音频格式重采样失败
    kOutOfMemory             //内存不足
};

//using PlayErrorCode = int;

struct PlayError
{
    PlayErrorCode code = PlayErrorCode::kNoError;
    std::string msg;
};

using VideoFormat = int;//对应ffmpeg中AVPixelFormat的值
using AudioFormat = int;//对应ffmpeg中AVSampleFormat的值

/*
* 视频规格
*/
struct VideoSpec
{
    uint16_t width = 0; //宽
    uint16_t height = 0;//高
    VideoFormat format = -1;//对应ffmpeg中AVPixelFormat的值
};


/*
* 音频规格
*/
struct AudioSpec
{
    uint16_t sampleRate = 0;  //采样率:8000,16000,44100等
    uint16_t bitPerSample = 0;//位深:8,16,32等
    uint16_t channels = 0;    //通道数:1,2等
    AudioFormat format = -1;  //对应ffmpeg中AVSampleFormat的值
};

/*
* 视频帧
*/
struct VideoFrame
{
    uint8_t*  data = nullptr;
    size_t    size = 0;
    int64_t   pts = 0;
    double    timebase = 0.0;
    VideoSpec spec;
};

/*
* 音频帧
*/
struct AudioFrame
{
    uint8_t*  data = nullptr;
    size_t    size = 0;
    int64_t   pts = 0;
    double    timebase = 0.0;
    AudioSpec spec;
};

struct MediaParameter
{
    std::string url;
    std::string hwDeviceName;
    VideoSpec outputVideoSpec;
    AudioSpec outputAudioSpec;
};

class PlayEvent
{
public:
    virtual ~PlayEvent() {};
    // 视频帧(渲染)回调
    virtual void onVideoFrame(const VideoFrame& frame) = 0;
    virtual void onClose(const PlayError& error) = 0;
private:

};

using VideoCallback = std::function<void(const VideoFrame& frame)>;
using AudioCallback = std::function<void(const AudioFrame& frame)>;

#endif // MEDIA_DEFINE_H
