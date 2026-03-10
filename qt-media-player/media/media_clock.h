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
    MediaClock();
    ~MediaClock();

    bool Wait(double pts, double master = -1.0);
    bool Wait2(double pts, double master = -1.0, double speed = 1.0);
    double GetClock();
    void SetClockAt(double pts, double time);
    void SetClock(double pts);
    void SetClockSpeed(double speed);
    void SyncClockToSlave(double slave_clock);
    void InitClock();
private:
    double ComputeTargetDelay(double delay, double pts, double master);
private:
    std::mutex m_mutex;
    double m_startTs = 0.0;   //基于时钟的初始时间
    double m_startPts = 0.0;  //初始的pts时间
    double m_lastPts = 0.0;   //上次的pts时间
    double m_lastDelay = 0.0; //上次的帧延迟时间
    double m_frameTimer = 0.0; // 基于cpu时间的帧时间

    double m_pts = NAN;
    double m_lastUpdated = 0.0;
    double m_ptsDrift = 0.0;//pts基于系统时钟(av_gettime_relative)的偏移量，m_pts - m_lastUpdated
    double m_speed = 1.0;
    int m_paused = 0;
};

} // namespace mp

#endif // MEDIA_CLOCK_H
