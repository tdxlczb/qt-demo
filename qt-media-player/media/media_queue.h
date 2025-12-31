#ifndef MEDIA_QUEUE_H
#define MEDIA_QUEUE_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include "media_define.h"

struct AVPacket;
struct AVFrame;

namespace mp {

class PacketQueue
{
public:
    PacketQueue(size_t maxQueueSize = 1000);
    ~PacketQueue();

    void Push(AVPacket* pkt, bool fullRemove = false); //满了移除队列
    AVPacket* PopFront();
    size_t Size();
    void Clear();
    void Resize(size_t maxQueueSize);
private:
    std::queue<AVPacket*> m_packetQueue;
    size_t m_maxQueueSize = 0;
    size_t m_maxWaitSize = 10;
    std::mutex m_packetQueueMutex;
    std::condition_variable m_queueCV;
};

class FrameQueue
{
public:
    FrameQueue(size_t maxQueueSize = 25);
    ~FrameQueue();

    void Push(AVFrame* frame);
    AVFrame* PopFront();
    size_t Size();
    void Clear();
    void Resize(size_t maxQueueSize);
private:
    std::queue<AVFrame*> m_frameQueue;
    size_t m_maxQueueSize = 0;
    std::mutex m_frameQueueMutex;
    std::condition_variable m_queueCV;
};

} // namespace mp

#endif // MEDIA_QUEUE_H
