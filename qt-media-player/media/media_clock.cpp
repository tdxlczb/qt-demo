#include "media_clock.h"
#include "log.h"
extern "C"
{
//#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

namespace mp {

MediaClock::MediaClock()
{
}

MediaClock::~MediaClock()
{
}

const double kMinSyncThreshold = 0.04; // 最小同步阈值
const double kMaxSyncThreshold = 0.1; // 最大同步阈值
const double kFrameDuplicationThreshold = 0.1;// 帧重复阈值,AV_SYNC_FRAMEDUP_THRESHOLD
const double kNoSyncThreshold = 10.0; // 不同步阈值
const double kMaxFrameDuration = 10.0; //最大帧播放时间
const double kRefreshTime = 0.01; //刷新时间

//参考fflay的写法
double MediaClock::ComputeTargetDelay(double delay, double pts, double master)
{
    double sync_threshold, diff = 0.0;
    // pts表示当前的时钟，master表示用来同步的时钟，master为-1.0时，不进行同步
    if (master > 0.0) {
        // get_clock(&is->vidclk)是获取到当前的视频时钟，视频时钟 = 当前正在播放帧的pts + 当前播放帧已经播放了的时间。
        // get_master_clock(is)是获取到当前的音频时钟（在视频同步到音频方法的时候），
        // 音频时钟 = 当前正在播放音频帧的播放结束时间 - 还未播放完的音频时长。
        // diff等于视频时钟相比音频时钟的差值；
        // diff > 0 表示视频快了；
        // diff < 0 表示视频慢了。
        // diff = get_clock(&is->vidclk) - get_master_clock(is);
        diff = pts - master;

        // delay就是last_duration，也就是当前播放帧理论应该播放的时长。
        // sync_threshold是视频时钟和音频时钟不同步的阈值，就取为delay也就是last_duration的值，并且在0.04到0.1秒之间。
        // 如果-sync_threshold < diff < sync_threshold的话就不需要调整last_duration了。
        sync_threshold = (std::max)(kMinSyncThreshold, (std::min)(kMaxSyncThreshold, delay));
        if (!isnan(diff) && fabs(diff) < kMaxFrameDuration) {

            // 如果视频时钟比音频时钟慢了的时间超过了sync_threshold，则将delay（也就是last_duration）减小diff，加快视频的速度。
            if (diff <= -sync_threshold)
                delay = (std::max)(0.0, delay + diff);

            // 如果视频时钟比音频时钟快了的时间超过了sync_threshold，并且delay（也就是last_duration）太长了，
            // 大于0.1秒（AV_SYNC_FRAMEDUP_THRESHOLD）的话，
            // 我们就直接将delay（也就是last_duration）增加一个diff，减慢视频的速度。
            else if (diff >= sync_threshold && delay > kFrameDuplicationThreshold)
                delay = delay + diff;

            // 如果视频时钟比音频时钟快了的时间超过了sync_threshold，并且delay（也就是last_duration）不怎么长的话，
            // 我们就将delay（也就是last_duration）增加一倍，减慢视频的速度。
            // 这里和前一个条件处理的不同就在于delay（也就是last_duration）是不是大于AV_SYNC_FRAMEDUP_THRESHOLD，
            // 上面不直接将delay翻倍应该是delay太大，大于了0.1秒了，超过了不同步阈值的最大值0.1秒了，还不如diff有多少就加多少。
            // 而这个条件里面delay翻倍而直接不增加diff的原因应该是一般帧率大概在20fps左右，last_duration差不多就0.05秒，
            // 增加一倍也不会太大，毕竟音视频同步本来就是动态同步。
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }
    return delay;
}

bool MediaClock::Wait(double pts, double master)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_startTs == 0.0 && !isnan(pts) && !isnan(master)) {
        m_startTs = master;
        m_startPts = pts;
    }

    double delay = pts - m_lastPts;
    if (isnan(delay) || delay <= 0 || delay > kMaxFrameDuration) {
        delay = m_lastDelay;
    }

    // last_duration是lastvp也就是当前正在播放的视频帧的理论应该播放的时间，
    // last_duration = vp->pts - lastvp->pts。
    double last_duration = delay;
    // compute_target_delay根据视频和音频的不同步情况，调整当前正在播放的视频帧的播放时间last_duration，
    // 得到实际应该播放的时间delay
    delay = ComputeTargetDelay(last_duration, pts, master);

    //delay /= speed;
    //if (pts < m_lastPts)
    //    return true;

    double time = av_gettime_relative() / 1000000.0;
    double actual_delay = m_frameTimer + delay - time;
    // is->frame_timer是当前正在播放视频帧应该开始播放的时间，
    // is->frame_timer + delay是当前正在播放视频帧经过音视频同步之后应该结束播放的时间，也就是下一帧应该开始播放的时间，
    // 如果当前时间time还没有到当前播放视频帧的结束时间的话，继续播放当前帧，并计算当前帧还需要播放多长时间remaining_time。
    if (actual_delay > 0.0) {
        double remaining_time = (std::min)(actual_delay, kRefreshTime);
        lock.unlock();
        av_usleep((int64_t)(remaining_time * 1000000.0));
        return false;
    }

    m_lastDelay = delay;
    m_lastPts = pts;
    // 如果当前正在播放的视频帧的播放时间已经足够了，那就播放下一帧，并更新is->frame_timer的值。
    m_frameTimer += delay;
    if (delay > 0.0 && time - m_frameTimer > kMaxSyncThreshold)
        m_frameTimer = time;

    return true;
}

bool MediaClock::Wait2(double pts, double master, double speed)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_startTs == 0.0 && !isnan(pts) && !isnan(master)) {
        m_startTs = master;
        m_startPts = pts;
    }
    double elapsedPts = pts - m_startPts; // 相对 pts
    double elapsedClock = master - m_startTs; // 相对 clock
    double diff = elapsedPts - elapsedClock;// +1.0; // > 0 表示视频超前, 增加1s延迟，会导致第一帧播放慢

    if (diff > 0) {
        diff = (std::min)(diff, kNoSyncThreshold); //避免等待时间过长
        av_usleep(diff * 1000000.0);
    }
    return true;
}

double MediaClock::GetClock()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_paused) {
        return m_pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        double clock = m_ptsDrift + time - (time - m_lastUpdated) * (1.0 - m_speed);
        return clock;
    }
}

void MediaClock::SetClockAt(double pts, double time)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pts = pts;
    m_lastUpdated = time;
    m_ptsDrift = pts - time;
}

void MediaClock::SetClock(double pts)
{
    double time = av_gettime_relative() / 1000000.0;
    SetClockAt(pts, time);
}

void MediaClock::SetClockSpeed(double speed)
{
    SetClock(GetClock());
    std::lock_guard<std::mutex> lock(m_mutex);
    m_speed = speed;
}

void MediaClock::SyncClockToSlave(double slave_clock)
{
    double clock = GetClock();
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > kNoSyncThreshold))
        SetClock(slave_clock);
}

void MediaClock::InitClock() {
    SetClock(NAN);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_startTs = 0.0;
    m_startPts = 0.0;
    m_lastPts = 0.0;
    m_lastDelay = 0.0;
    m_frameTimer = 0.0;
    m_speed = 1.0;
    m_paused = 0;
}

} // namespace mp