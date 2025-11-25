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
    PacketQueue(int16_t maxQueueSize = 1000);
    ~PacketQueue();

    void Push(AVPacket* pkt, bool fullRemove = false); //满了移除队列
    AVPacket* PopFront();
    size_t Size();
    void Clear();
private:
    std::queue<AVPacket*> m_packetQueue;
    const int16_t m_maxQueueSize = 0;
    const int16_t m_maxWaitSize = 10;
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
    size_t Size();
    void Clear();
private:
    std::queue<AVFrame*> m_frameQueue;
    const int16_t m_maxQueueSize = 0;
    std::mutex m_frameQueueMutex;
    std::condition_variable m_queueCV;
};

#endif // MEDIA_QUEUE_H
