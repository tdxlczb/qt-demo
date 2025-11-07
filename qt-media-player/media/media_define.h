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
    int width = 0; //宽
    int height = 0;//高
    VideoFormat format = -1;//对应ffmpeg中AVPixelFormat的值
};

/*
* 音频规格
*/
struct AudioSpec
{
    int sampleRate = 0;  //采样率:8000,16000,44100等
    int bitPerSample = 0;//位深:8,16,32等
    int channels = 0;    //通道数:1,2等
    AudioFormat format = -1;  //对应ffmpeg中AVSampleFormat的值
};

using CopyFrameCallback = std::function<int(
    uint8_t* dst_data[4], int dst_linesizes[4],
    uint8_t* src_data[4], int src_linesizes[4],
    int pix_fmt, int width, int height)>;

/*
* 视频帧
*/
struct VideoFrame
{
    uint8_t*  data = nullptr;
    size_t    size = 0;
    uint8_t*  linedata[8] = { 0 };
    int       linesize[8] = { 0 };
    CopyFrameCallback copycb;
    double    pts = 0.0;//帧时间，转成秒
    size_t    index = 0;
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

struct PlayOptions
{
    std::string hwdevice;
};

const int kRenderFmtNONE = -1; //AV_PIX_FMT_NONE
const int kRenderFmtRGB = 2; //AV_PIX_FMT_RGB24
const int kRenderFmtYUV420P = 0; //AV_PIX_FMT_YUV420P
const int kRenderFmtYUVJ420P = 12; //AV_PIX_FMT_YUVJ420P
const int kRenderFmtNV12 = 23; //AV_PIX_FMT_NV12

using VideoCallback = std::function<void(const VideoFrame& frame)>;
using AudioCallback = std::function<void(const AudioFrame& frame)>;


#endif // MEDIA_DEFINE_H
