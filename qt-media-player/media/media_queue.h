#ifndef MEDIA_QUEUE_H
#define MEDIA_QUEUE_H

#include <queue>
#include <condition_variable>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "media_define.h"


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
    FrameQueue(int16_t maxQueueSize = 5);
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

#endif // MEDIA_QUEUE_H
