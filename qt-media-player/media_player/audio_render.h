#ifndef AUDIORENDER_H
#define AUDIORENDER_H

#include <condition_variable>
#include "utils/jitter_buffer.h"
#include "media/media_define.h"


class RtAudio;
class AudioRender
{
public:
    AudioRender();
    ~AudioRender();

    void Start(int16_t sampleRate, int16_t bitPerSample, int16_t channelCount, int16_t renderFrameCount);
    void Stop();
    void Write(const AudioFrame& frame);

    //设置音量，音量范围在0.0~1.0
    void SetVolume(float volume);// 0.0~1.0
    float GetVolume();

    int AudioOutputCallback(void* outputBuffer, unsigned int nFrames);

private:
    DynamicJitterBuffer* m_audioBuffer = nullptr;
    std::mutex m_audioBufferMutex;
    std::condition_variable m_audioBufferCV;
    RtAudio* m_rtAudio = nullptr;
    int16_t m_sampleRate = 0;
    int16_t m_bitPerSample = 0;
    int16_t m_channelCount = 0;
    int16_t m_renderFrameCount = 0;
    float m_audioVolume = 1.0;
};

#endif // AUDIORENDER_H
