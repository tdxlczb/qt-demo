#ifndef MEDIA_CLOCK_H
#define MEDIA_CLOCK_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include <cmath>

namespace mp {

//参考fflay的时钟写法
//ffplay的倍速播放是通过滤镜实现的，时钟设置的倍速好像没有使用
class MediaClock
{
public:
    MediaClock(const std::string& name = "");
    ~MediaClock();

    // 等待直到帧应该显示的时间，返回true表示可以显示，false表示还需要继续等待
    // pts表示视频帧的pts，clock表示视频时钟，master表示主时钟，mater<0表示不同步
    bool Wait(double pts, double clock, double master);
    bool Wait2(double pts, double master = -1.0, double speed = 1.0);
    // 获取当前时钟时间
    double GetClock();
    // 基于指定系统时间设置时钟PTS
    void SetClockAt(double pts, double time);
    // 设置时钟PTS
    void SetClock(double pts);
    // 设置播放速度
    void SetClockSpeed(double speed);
    // 同步到从时钟：当偏差超过阈值时强制同步
    void SyncClockToSlave(double slave_clock);
    // 初始化时钟
    void InitClock(const std::string& name = "");
private:
    // 计算考虑同步后的目标延迟
    double ComputeTargetDelay(double delay, double clock, double master);
    // 获取当前系统时间
    double GetSystemTime();
private:
    std::string m_name;
    std::mutex m_mutex;

    // 帧计时相关
    double m_frameTimer = 0.0;        // 帧播放计时器（系统时钟时间）
    double m_lastPts = NAN;           // 上一帧的PTS
    double m_lastDelay = NAN;         // 上一帧的实际延迟

    // 时钟相关
    double m_pts = NAN;               // 当前时钟PTS
    double m_lastUpdated = 0.0;       // 最后一次更新时钟的系统时间
    double m_ptsDrift = 0.0;          // PTS相对于系统时钟的漂移量
    double m_speed = 1.0;             // 播放速度
    bool m_paused = false;            // 是否暂停

    double m_startTs = 0.0;           // 开始时的主时钟时间
    double m_startPts = 0.0;          // 开始时的PTS
};


} // namespace mp

#endif // MEDIA_CLOCK_H
