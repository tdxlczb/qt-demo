#ifndef MEDIA_AUDIO_FILTER_H
#define MEDIA_AUDIO_FILTER_H

#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS

#include <mutex>

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
}

class AudioSpeedFilter
{
public:
    AudioSpeedFilter();
    ~AudioSpeedFilter();

    // 初始化滤镜图
    bool Init(int sampleRate, AVSampleFormat sampleFmt, int64_t channelLayout, double speed);
    // 清理资源
    void UnInit();
    // 主动态改变倍速
    bool ChangeSpeed(double newSpeed);
    // 使用命令模式主动改变倍速
    bool ChangeSpeedByCommond(double newSpeed);
    // 发送音频帧到滤镜
    bool SendFrame(AVFrame* frame);
    // 从滤镜获取处理后的帧
    AVFrame* ReceiveFrame();

private:
    AVFilterGraph* m_filterGraph = nullptr;
    AVFilterContext* m_inFilterCtx = nullptr;    // abuffer（输入）
    AVFilterContext* m_outFilterCtx = nullptr;   // abuffersink（输出）
    AVFilterContext* m_atempoCtx = nullptr;      // atempo（倍速）
    bool m_isInitialized = false;
    std::mutex m_mutex;

    int m_sampleRate = 0;
    AVSampleFormat m_sampleFmt;
    int64_t m_channelLayout = 0;
    double m_speed = 1.0;
};

void processAudioFrame(AVFrame* decodedFrame, AudioSpeedFilter* filter);

#endif // MEDIA_AUDIO_FILTER_H
