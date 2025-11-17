#ifndef MEDIA_DISPLAY_H
#define MEDIA_DISPLAY_H

#include <mutex>
#include "media_define.h"

struct AVFrame;
struct SwsContext;
struct SwrContext;
/*
* 视频格式转换器
*/
class VideoConverter
{
public:
    VideoConverter(int converterId, const VideoSpec& dstSpec);
    ~VideoConverter();
    /*
    * 设置回调函数
    */
    void SetCallback(const VideoCallback& callback);
    /*
    * 重采样，并且调用回调函数，可以直接获取到重采样后的帧
    * 返回的帧数据使用的是当前类的buffer地址，会被下一帧数据覆盖，使用data不安全，更多的是是使用其他信息
    */
    PlayError DisplayInput(AVFrame* pFrame, VideoFrame& outFrame);
    PlayError DisplayInput(const VideoFrame& inFrame, VideoFrame& outFrame);
    /*
    * 重采样，并且调用回调函数
    */
    PlayError DisplayInput(AVFrame* pFrame);
    /*
    * 更新播放大小，为避免快速更改大小导致的重新初始化重采样器，每次更改重采样器需要间隔1s
    */
    void UpdateDisplaySize(int iDisplayWidth, int iDisplayHeight);
private:
    PlayError InitSwsContext(const VideoSpec& srcSpec);
    PlayError InitDstFrame();
private:
    int m_converterId = 0;
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
    AudioConverter(int converterId, const AudioSpec& dstSpec);
    ~AudioConverter();
    /*
    * 设置回调函数
    */
    void SetCallback(const AudioCallback& callback);
    /*
    * 重采样，并且调用回调函数，可以直接获取到重采样后的帧
    * 返回的帧数据使用的是当前类的buffer地址，会被下一帧数据覆盖，如果要使用data
    */
    PlayError DisplayInput(AVFrame* pFrame, AudioFrame& outFrame);
    PlayError DisplayInput(const AudioFrame& inFrame, AudioFrame& outFrame);
    /*
    * 重采样，并且调用回调函数
    */
    PlayError DisplayInput(AVFrame* pFrame);
    /*
    * 获取结尾数据
    */
    PlayError GetDisplayEndFrame(AudioFrame& outFrame);
private:
    PlayError InitSwrContext(const AudioSpec& srcSpec);
    PlayError InitDstFrame();
private:
    int m_converterId = 0;
    AudioSpec m_srcSpec;
    AudioSpec m_dstSpec;
    AudioCallback m_pCallback;
    AVFrame* m_pFrameDst = nullptr; //重采样保存的帧，放到成员中减少申请和消耗内存的性能消耗
    SwrContext* m_pSwrCxtAudio = nullptr;
    std::mutex m_swsMutex; //重采样操作锁
};

#endif // MEDIA_DISPLAY_H
