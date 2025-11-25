#ifndef MEDIA_CONVERTER_H
#define MEDIA_CONVERTER_H

#include <mutex>
#include "media_define.h"

struct AVFrame;
struct SwsContext;
struct SwrContext;
/*
* 视频格式转换器，视频转换一定要保证源宽高和目标宽高比例一致，要是不一致，重采样出来的图片是混乱的
* 转换器初始化宽高为0时，默认使用源宽高，初始化格式为-1时，默认使用源格式，视频规格前后源和目标一致，不进行格式转换
*/
class VideoConverter
{
public:
    VideoConverter(const VideoSpec& dstSpec, const std::string& id = "VideoConverter");
    ~VideoConverter();
    /*
    * 获取转换格式
    */
    VideoSpec GetSrcSpec();
    VideoSpec GetDstSpec();
    /*
    * 设置回调函数
    */
    void SetCallback(const VideoCallback& callback);
    /*
    * 重采样，并且调用回调函数，可以直接获取到重采样后的帧
    * 返回的帧数据使用的是当前类的buffer地址，会被下一帧数据覆盖，使用data不安全，更多的是是使用其他信息
    */
    PlayError Input(AVFrame* pFrame, VideoFrame& outFrame);
    PlayError Input(const VideoFrame& inFrame, VideoFrame& outFrame);
    /*
    * 重采样，并且调用回调函数
    */
    PlayError Input(AVFrame* pFrame);
    /*
    * 更新目标大小，为避免快速更改大小导致的重新初始化重采样器，每次更改重采样器需要间隔1s
    */
    void UpdateDstSize(int iDstWidth, int iDstHeight);

private:
    PlayError InitSwsContext(const VideoSpec& srcSpec);
    PlayError InitDstFrame();
private:
    const std::string m_converterId;
    VideoSpec m_srcSpec;
    VideoSpec m_dstSpec;
    VideoCallback m_pCallback;
    AVFrame* m_pFrameDst = nullptr; //重采样保存的帧，放到成员中减少申请和消耗内存的性能消耗
    SwsContext* m_pSwsCxtVideo = nullptr;
    std::mutex m_swsMutex; //重采样操作锁
    int64_t m_lastUpdateSizeTime = 0;//记录上次更新大小的事件，避免快速更改大小导致的重新初始化重采样器
};

/*
* 音频格式转换器
*/
class AudioConverter
{
public:
    AudioConverter(const AudioSpec& dstSpec, const std::string& id = "AudioConverter");
    ~AudioConverter();
    /*
    * 获取转换格式
    */
    AudioSpec GetSrcSpec();
    AudioSpec GetDstSpec();
    /*
    * 设置回调函数
    */
    void SetCallback(const AudioCallback& callback);
    /*
    * 重采样，并且调用回调函数，可以直接获取到重采样后的帧
    * 返回的帧数据使用的是当前类的buffer地址，会被下一帧数据覆盖，如果要使用data
    */
    PlayError Input(AVFrame* pFrame, AudioFrame& outFrame);
    PlayError Input(const AudioFrame& inFrame, AudioFrame& outFrame);
    /*
    * 重采样，并且调用回调函数
    */
    PlayError Input(AVFrame* pFrame);
    /*
    * 获取结尾数据
    */
    PlayError GetEndFrame(AudioFrame& outFrame);
private:
    PlayError InitSwrContext(const AudioSpec& srcSpec);
    PlayError InitDstFrame();
private:
    const std::string m_converterId;
    AudioSpec m_srcSpec;
    AudioSpec m_dstSpec;
    AudioCallback m_pCallback;
    AVFrame* m_pFrameDst = nullptr; //重采样保存的帧，放到成员中减少申请和消耗内存的性能消耗
    SwrContext* m_pSwrCxtAudio = nullptr;
    std::mutex m_swsMutex; //重采样操作锁
};

#endif // MEDIA_CONVERTER_H
