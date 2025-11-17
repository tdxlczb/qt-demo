#include "media_queue.h"
#include "log.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

PacketQueue::PacketQueue(int16_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize)
{
}

PacketQueue::~PacketQueue()
{
}

void PacketQueue::Push(AVPacket* pkt, bool fullRemove)
{
    AVPacket* pktNew = av_packet_alloc();
    if (!pktNew) {
        av_packet_unref(pkt);
        return;
    }
    av_packet_move_ref(pktNew, pkt);

    std::unique_lock<std::mutex> lock(m_packetQueueMutex);
    if (!fullRemove) {
        // 当队列满了，阻塞在这里
        // 停止播放时，通过Clear清空队列可以解除阻塞
        m_queueCV.wait(lock, [this]() {
            return m_packetQueue.size() < m_maxQueueSize;
            });
        //lock.unlock();
        m_packetQueue.push(pktNew);
    }
    else {
        if (m_packetQueue.size() < m_maxQueueSize) {
            m_packetQueue.push(pktNew);
        }
        else {
            LOG_INFO << "packet 队列已满，准备清空";
            //这里清空队列要释放内存，清理一半
            auto packetSize = m_packetQueue.size() / 2;
            for (size_t i = 0; i < packetSize; i++)
            {
                AVPacket* packet = m_packetQueue.front();
                if (i == 0) {
                    LOG_INFO << "start pop packet pts:" << packet->pts;
                }
                if (i == packetSize - 1) {
                    LOG_INFO << "end pop packet pts:" << packet->pts;
                }
                m_packetQueue.pop();
                av_packet_free(&packet);
            }
        }
    }
}

AVPacket* PacketQueue::PopFront()
{
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    if (m_packetQueue.empty())
        return nullptr;
    AVPacket* pkt = m_packetQueue.front();
    m_packetQueue.pop();
    m_queueCV.notify_all();
    return pkt;
}

size_t PacketQueue::Size()
{
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    return m_packetQueue.size();
}

void PacketQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_packetQueueMutex);
    while (!m_packetQueue.empty())
    {
        AVPacket* packet = m_packetQueue.front();
        m_packetQueue.pop();
        av_packet_free(&packet);
    }
}

FrameQueue::FrameQueue(int16_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize)
{
}

FrameQueue::~FrameQueue()
{
}

void FrameQueue::Push(AVFrame* frame)
{
    std::unique_lock<std::mutex> lock(m_frameQueueMutex);
    // 当队列满了，阻塞在这里
    // 停止播放时，通过Clear清空队列可以解除阻塞
    m_queueCV.wait(lock, [this]() {
        return m_frameQueue.size() < m_maxQueueSize;
        });
    //lock.unlock();
    m_frameQueue.push(frame);
}

AVFrame* FrameQueue::PopFront()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    if (m_frameQueue.empty())
        return nullptr;
    AVFrame* frame = m_frameQueue.front();
    m_frameQueue.pop();
    m_queueCV.notify_all();
    return frame;
}

size_t FrameQueue::Size()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    return m_frameQueue.size();
}

void FrameQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    //std::queue<AVFrame*> empty;
    //std::swap(empty, m_frameQueue);
    while (!m_frameQueue.empty())
    {
        AVFrame* frame = m_frameQueue.front();
        m_frameQueue.pop();
        av_frame_free(&frame);
    }
}


QueueClock::QueueClock(double v = 0.0)
    : frameRate(v)
{
}

QueueClock::~QueueClock()
{
}

bool QueueClock::wait(bool shouldSync, double pts, double speed, double master)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    double delay = pts - prevPts;
    if (isnan(delay) || delay <= 0 || delay > maxFrameDuration)
        delay = frameRate;

    if (master > 0) {
        double diff = pts - master;
        double sync_threshold = qMax(minThreshold, qMin(maxThreshold, delay));
        if (!isnan(diff) && fabs(diff) < maxFrameDuration) {
            if (diff <= -sync_threshold)
                delay = qMax(0.0, delay + diff);
            else if (diff >= sync_threshold && delay > frameDuplicationThreshold)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    delay /= speed;
    const double time = av_gettime_relative() / 1000000.0;
    if (shouldSync) {
        if (pts < prevPts)
            return true;
        if (time < frameTimer + delay) {
            double remaining_time = qMin(frameTimer + delay - time, refreshRate);
            locker.unlock();
            av_usleep((int64_t)(remaining_time * 1000000.0));
            return false;
        }
    }

    prevPts = pts;
    frameTimer += delay;
    if ((delay > 0 && time - frameTimer > maxThreshold) || !shouldSync)
        frameTimer = time;

    return true;
}

double QueueClock::pts()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return prevPts;
}

void QueueClock::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    prevPts = 0;
    frameTimer = 0;
}

void QueueClock::setFrameRate(double v)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    frameRate = v;
}