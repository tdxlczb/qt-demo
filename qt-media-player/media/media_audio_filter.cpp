#include "media_audio_filter.h"
#include "media_utils.h"

namespace mp {

AudioSpeedFilter::AudioSpeedFilter()
{
}

AudioSpeedFilter::~AudioSpeedFilter()
{
    UnInit();
}

// 初始化滤镜图
bool AudioSpeedFilter::Init(int sampleRate, AVSampleFormat sampleFmt, int64_t channelLayout, double speed) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // 1. 创建滤镜图
    m_filterGraph = avfilter_graph_alloc();
    if (!m_filterGraph) return false;

    // 2. 创建 abuffer 滤镜（接收解码后的音频帧）
    const AVFilter* abuffer = avfilter_get_by_name("abuffer");
    if (!abuffer) return false;

    // 设置音频参数：采样率、格式、声道布局
    char args[512];
    snprintf(args, sizeof(args), "time_base=%d/1:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
        1, sampleRate, av_get_sample_fmt_name(sampleFmt), channelLayout);

    int ret = avfilter_graph_create_filter(&m_inFilterCtx, abuffer, "in", args, nullptr, m_filterGraph);
    if (ret < 0) return false;

    // 3. 创建 atempo 滤镜（核心倍速）
    const AVFilter* atempo = avfilter_get_by_name("atempo");
    if (!atempo) return false;

    // 设置倍速参数（0.5~2.0，超过需串联多个）
    char speedStr[32];
    snprintf(speedStr, sizeof(speedStr), "%.2f", speed);

    ret = avfilter_graph_create_filter(&m_atempoCtx, atempo, "atempo", speedStr, nullptr, m_filterGraph);
    if (ret < 0) return false;

    // 4. 创建 abuffersink 滤镜（获取处理后的帧）
    const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
    if (!abuffersink) return false;

    ret = avfilter_graph_create_filter(&m_outFilterCtx, abuffersink, "out", nullptr, nullptr, m_filterGraph);
    if (ret < 0) return false;

    // 5. 连接滤镜：abuffer -> atempo -> abuffersink
    ret = avfilter_link(m_inFilterCtx, 0, m_atempoCtx, 0);
    if (ret < 0) return false;

    ret = avfilter_link(m_atempoCtx, 0, m_outFilterCtx, 0);
    if (ret < 0) return false;

    // 6. 配置滤镜图
    ret = avfilter_graph_config(m_filterGraph, nullptr);
    if (ret < 0) return false;

    m_sampleRate = sampleRate;
    m_sampleFmt = sampleFmt;
    m_channelLayout = channelLayout;
    m_speed = speed;
    m_isInitialized = true;
    return true;
}

// 清理资源
void AudioSpeedFilter::UnInit() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_filterGraph) {
        avfilter_graph_free(&m_filterGraph); // 会自动释放所有滤镜上下文
    }
    //AVFilterContext的生命周期由AVFilterGraph统一管理
    m_inFilterCtx = m_outFilterCtx = m_atempoCtx = nullptr;
    m_isInitialized = false;
}

// 主动态改变倍速
bool AudioSpeedFilter::ChangeSpeed(double newSpeed)
{
    if (newSpeed < 0.5 || newSpeed > 2.0) return false;

    if (!m_isInitialized || !m_atempoCtx) return false;

    // 销毁旧滤镜图
    UnInit();

    // 重新创建滤镜图
    return Init(m_sampleRate, m_sampleFmt, m_channelLayout, newSpeed);
}

bool AudioSpeedFilter::ChangeSpeedByCommond(double newSpeed)
{
    if (newSpeed < 0.5 || newSpeed > 2.0) return false;

    if (!m_isInitialized || !m_atempoCtx) return false;


    std::lock_guard<std::mutex> lock(m_mutex);

    // 构造命令
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "%.2f", newSpeed);

    // 发送 speed 命令到 atempo 滤镜
    // 关键参数：
    // - graph: 滤镜图
    // - target: 滤镜实例名（初始化时的 "atempo"）
    // - cmd: 命令名称 "speed"
    // - arg: 参数值
    // - res: 返回结果缓冲区（不需要设为nullptr）
    // - res_len: 缓冲区长度（0表示不接收返回）
    // - flags: 标志位（0）
    int ret = avfilter_graph_send_command(
        m_filterGraph,        // 滤镜图
        "atempo",           // 滤镜实例名（与init时一致）
        "speed",            // 命令名称（atempo支持的命令）
        cmd,                // 参数值字符串
        nullptr,            // 返回缓冲区（不需要）
        0,                  // 缓冲区长度
        0                   // 标志位
    );

    if (ret < 0) {
        char errBuf[128];
        av_strerror(ret, errBuf, sizeof(errBuf));
        //printf("发送速度命令失败: %s\n", errBuf);
        return false;
    }

    m_speed = newSpeed;
    //printf("音频倍速已动态调整为: %.2fx\n", newSpeed);
    return true;
}

// 发送音频帧到滤镜
bool AudioSpeedFilter::SendFrame(AVFrame* frame) {
    if (!m_isInitialized || !m_inFilterCtx) return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    int ret = av_buffersrc_add_frame(m_inFilterCtx, frame);
    return ret >= 0;
}

// 从滤镜获取处理后的帧
AVFrame* AudioSpeedFilter::ReceiveFrame() {
    if (!m_isInitialized || !m_outFilterCtx) return nullptr;

    std::lock_guard<std::mutex> lock(m_mutex);
    AVFrame* filteredFrame = av_frame_alloc();
    int ret = av_buffersink_get_frame(m_outFilterCtx, filteredFrame);

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        av_frame_free(&filteredFrame);
        return nullptr; // 需要更多输入或已经结束
    }
    if (ret < 0) {
        av_frame_free(&filteredFrame);
        return nullptr; // 错误
    }

    return filteredFrame; // 返回倍速后的音频帧
}

//static AudioPCMWriter g_filterOutput("audio_filter_output.pcm");

void processAudioFrame(AVFrame* decodedFrame, AudioSpeedFilter* filter) {
    // 1. 发送解码帧到滤镜
    filter->SendFrame(decodedFrame);

    // 2. 循环获取倍速后的帧（可能1输入对应多输出或少输出）
    while (true) {
        AVFrame* filteredFrame = filter->ReceiveFrame();
        if (!filteredFrame) break; // 没有更多输出

        // 3. 将 filteredFrame 送入音频播放设备
        // audioDevice.play(filteredFrame);

        size_t bufferSize = av_samples_get_buffer_size(filteredFrame->linesize, filteredFrame->channels, filteredFrame->nb_samples, (AVSampleFormat)filteredFrame->format, 1);
        //g_filterOutput.Write(reinterpret_cast<const char*>(filteredFrame->data[0]), bufferSize);

        // 4. 释放处理后的帧
        av_frame_free(&filteredFrame);
    }
}

} // namespace mp