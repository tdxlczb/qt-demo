#ifndef AUDIO_RENDER_H
#define AUDIO_RENDER_H

#include <condition_variable>
#include <functional>
#include "utils/jitter_buffer.h"
#include "media/media_define.h"

using PCMCallback = std::function<size_t(uint8_t* buffer, size_t len)>;


class RtAudio;
class AudioRender
{
public:
    AudioRender(bool useJitterBuffer = false);
    ~AudioRender();

    void Start(const mp::AudioSpec& audioSpec, int renderFrameCount);
    void Stop();
    void SetPCMCallback(PCMCallback callback);

    void Write(const mp::AudioFrame& frame);

    //设置音量，音量范围在0.0~1.0
    void SetVolume(float volume);// 0.0~1.0
    float GetVolume();

    int AudioOutputCallback(void* outputBuffer, unsigned int nFrames);

private:
    RtAudio* m_rtAudio = nullptr;
    mp::AudioSpec m_audioSpec;
    int m_renderFrameCount = 0;
    float m_audioVolume = 1.0;
    PCMCallback m_pcmCallback;
    bool m_isUseJitterBuffer = false;
    DynamicJitterBuffer* m_audioBuffer = nullptr;
    std::mutex m_audioBufferMutex;
    std::condition_variable m_audioBufferCV;

};

#endif // AUDIO_RENDER_H
