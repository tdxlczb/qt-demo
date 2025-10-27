#include "media_queue.h"
#include <QDebug>

PacketQueue::PacketQueue(int16_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize)
{
}

PacketQueue::~PacketQueue()
{
}

void PacketQueue::Push(AVPacket* pkt)
{
    std::unique_lock<std::mutex> lock(m_packetQueueMutex);
    AVPacket* pktNew = av_packet_alloc();
    if (!pktNew) {
        av_packet_unref(pkt);
        return;
    }
    av_packet_move_ref(pktNew, pkt);
    if (m_packetQueue.size() < m_maxQueueSize) {
        m_packetQueue.push(pktNew);
    }
    else {
        qInfo() << "packet 队列已满，准备清空";
        //这里清空队列要释放内存
        auto packetSize = m_packetQueue.size();
        for (size_t i = 0; i < packetSize; i++)
        {
            AVPacket* packet = m_packetQueue.front();
            if (i == 0) {
                qInfo() << "start pop packet pts:" << packet->pts;
            }
            if (i == packetSize - 1) {
                qInfo() << "end pop packet pts:" << packet->pts;
            }
            m_packetQueue.pop();
            av_packet_free(&packet);
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
    return pkt;
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

void FrameQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_frameQueueMutex);
    std::queue<AVFrame*> empty;
    std::swap(empty, m_frameQueue);
}
