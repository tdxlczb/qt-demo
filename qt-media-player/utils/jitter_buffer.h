#ifndef JITTER_BUFFER_H
#define JITTER_BUFFER_H

#include <atomic>
#include <cstring>
#include <stdexcept>
#include <mutex>

#define MAX_BUFFER_SIZE 1024 * 1024

class BufferBase {
public:
    virtual ~BufferBase() = default;
    virtual size_t GetCapacity() = 0;
    virtual size_t GetSize() = 0;
    virtual size_t GetFreeSize() = 0;
    virtual size_t ResetCapacity(size_t size) = 0;
};

template <typename T>
class JitterBuffer : public BufferBase {
public:
    explicit JitterBuffer(size_t capacity = MAX_BUFFER_SIZE) {
        m_buffer = new T[capacity];
        std::memset(m_buffer, 0, capacity);
        m_capacity = capacity;
    }
    ~JitterBuffer() {
        delete[] m_buffer;
        m_buffer = nullptr;
    };

    size_t PushData(T* buffer, size_t size) {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        if (m_capacity < size)
            return 0;
        if (m_capacity < (m_bufferSize + size))
            return 0;
        std::memcpy(m_buffer + m_bufferSize / sizeof(T), buffer, size);
        m_bufferSize += size;
        return size;
    }

    size_t PopData(T* buffer, size_t size) {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        if (m_bufferSize <= 0)
            return 0;
        size_t popSize = (std::min)(m_bufferSize, size);
        if (buffer) {
            std::memcpy(buffer, m_buffer, popSize);
        }
        std::memmove(m_buffer, m_buffer + popSize / sizeof(T), m_bufferSize - popSize);
        m_bufferSize -= popSize;
        return popSize;
    }

    size_t ResetCapacity(size_t size) {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        if (m_capacity >= size)
            return m_capacity;

        T* buffer = new T[size];
        std::memset(buffer, 0, size);
        std::memcpy(buffer, m_buffer, m_bufferSize);
        m_capacity = size;
        delete[] m_buffer;
        m_buffer = buffer;
        return m_capacity;
    }

    size_t GetCapacity() {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        return m_capacity;
    }

    size_t GetSize() {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        return m_bufferSize;
    }

    size_t GetFreeSize() {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        return m_capacity - m_bufferSize;
    }

private:
    T* m_buffer;
    size_t m_capacity = 0;
    size_t m_bufferSize = 0;
    std::mutex m_bufferMutex;
};

class DynamicJitterBuffer
{
public:
    // 动态初始化 Buffer（在 new 时决定类型）
    template <typename T>
    void Init(size_t capacity = MAX_BUFFER_SIZE) {
        m_isInit.store(true);
        m_jitterBuffer = new JitterBuffer<T>(capacity);
    }
    void UnInit() {
        m_isInit.store(false);
        delete m_jitterBuffer;
        m_jitterBuffer = nullptr;
    }
    bool IsInit() {
        return m_isInit.load();
    }

    template <typename T>
    size_t PushData(T* buffer, size_t size)
    {
        if (auto derived = dynamic_cast<JitterBuffer<T>*>(m_jitterBuffer)) {
            return derived->PushData(buffer, size);
        } else {
            throw std::runtime_error("type mismatch");
        }
        return 0;
    }

    template <typename T>
    size_t PopData(T* buffer, size_t size)
    {
        if (auto derived = dynamic_cast<JitterBuffer<T>*>(m_jitterBuffer)) {
            return derived->PopData(buffer, size);
        } else {
            throw std::runtime_error("type mismatch");
        }
        return 0;
    }

    size_t ResetCapacity(size_t size)
    {
        return m_jitterBuffer->ResetCapacity(size);
    }

    size_t GetCapacity()
    {
        return m_jitterBuffer->GetCapacity();
    }

    size_t GetSize()
    {
        return m_jitterBuffer->GetSize();
    }

    size_t GetFreeSize()
    {
        return m_jitterBuffer->GetFreeSize();
    }

private:
    BufferBase* m_jitterBuffer = nullptr;
    std::atomic_bool m_isInit {false};
};


#endif // JITTER_BUFFER_H
