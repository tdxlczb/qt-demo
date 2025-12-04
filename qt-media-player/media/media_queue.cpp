#include "media_queue.h"
#include "log.h"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
}

namespace mp {

PacketQueue::PacketQueue(size_t maxQueueSize)
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
            return m_packetQueue.size() < m_maxWaitSize;
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
    m_queueCV.notify_all();
}


FrameQueue::FrameQueue(size_t maxQueueSize)
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
    m_queueCV.notify_all();
}

} // namespace mp