#ifndef MEDIA_QUEUE_H
#define MEDIA_QUEUE_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include "media_define.h"

struct AVPacket;
struct AVFrame;
class PacketQueue
{
public:
    PacketQueue(int16_t maxQueueSize = 50000);
    ~PacketQueue();

    void Push(AVPacket* pkt);
    AVPacket* PopFront();
private:
    std::queue<AVPacket*> m_packetQueue;
    const int16_t m_maxQueueSize = 0;
    std::mutex m_packetQueueMutex;
    std::condition_variable m_queueCV;
};

class FrameQueue
{
public:
    FrameQueue(int16_t maxQueueSize = 25);
    ~FrameQueue();

    void Push(AVFrame* frame);
    AVFrame* PopFront();
    void Clear();
private:
    std::queue<AVFrame*> m_frameQueue;
    const int16_t m_maxQueueSize = 0;
    std::mutex m_frameQueueMutex;
    std::condition_variable m_queueCV;
};

class QueueClock
{
public:
    QueueClock(double v);
    ~QueueClock();

    bool wait(bool shouldSync, double pts, double speed = 1.0, double master = -1);
    double pts();
    void clear();
    void setFrameRate(double v);

private:
    double frameRate = 0;
    double frameTimer = 0;
    double prevPts = 0;
    std::mutex m_mutex;
    const double maxFrameDuration = 10.0;
    const double minThreshold = 0.04;
    const double maxThreshold = 0.1;
    const double frameDuplicationThreshold = 0.1;
    const double refreshRate = 0.01;
};


#endif // MEDIA_QUEUE_H
