#ifndef AUDIORENDER_H
#define AUDIORENDER_H

#include <QObject>
#include "utils/jitter_buffer.h"
#include "media/media_define.h"


class RtAudio;
class AudioRender : public QObject
{
    Q_OBJECT
public:
    explicit AudioRender(QObject *parent = nullptr);
    ~AudioRender();

    void Start(int16_t sampleRate, int16_t bitPerSample, int16_t channelCount, int16_t renderFrameCount);
    void Stop();
    void Write(const AudioFrame&frame);
    void SetVolume(float volume);// 0.0~1.0
    float GetVolume();
    int AudioOutputCallback(void *outputBuffer, unsigned int nFrames);
signals:
private:

private:
    DynamicJitterBuffer* m_audioBuffer = nullptr;
    RtAudio* m_rtAudio = nullptr;
    int16_t m_sampleRate = 0;
    int16_t m_bitPerSample = 0;
    int16_t m_channelCount = 0;
    int16_t m_renderFrameCount = 0;
    float m_audioVolume = 1.0;
};

#endif // AUDIORENDER_H
