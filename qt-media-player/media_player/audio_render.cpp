#include "audio_render.h"
#include <QDebug>
#include "RtAudio.h"

//#define DEBUG_PCM
#ifdef DEBUG_PCM
#include <fstream>
std::ofstream g_pcmOutput;
std::ofstream g_pcmInput;
#endif

AudioRender::AudioRender(QObject *parent) 
    : QObject(parent)
    , m_audioBuffer(new DynamicJitterBuffer())
    , m_rtAudio(new RtAudio())
{

}

AudioRender::~AudioRender()
{

}

static int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, unsigned int status, void *userData)
{
    AudioRender* render = (AudioRender*)userData;
    if (!render)
        return 0;
    return render->AudioOutputCallback(outputBuffer,nFrames);
}

void AudioRender::Start(int16_t sampleRate, int16_t bitPerSample, int16_t channelCount, int16_t renderFrameCount)
{
    if (!m_audioBuffer->IsInit())
    {
        if (bitPerSample == 8)
            m_audioBuffer->Init<int8_t>();
        else if (bitPerSample == 16)
            m_audioBuffer->Init<int16_t>();
        else if (bitPerSample == 32)
            m_audioBuffer->Init<int32_t>();
    }

    RtAudio::StreamParameters params;
    params.deviceId = m_rtAudio->getDefaultOutputDevice(); // 默认输出设备
    params.nChannels = channelCount;                   // 立体声
    params.firstChannel = 0;                       // 起始声道

    RtAudioFormat audioFmt;
    if (bitPerSample == 8)
        audioFmt = RTAUDIO_SINT8;
    else if (bitPerSample == 16)
        audioFmt = RTAUDIO_SINT16;
    else if (bitPerSample == 32)
        audioFmt = RTAUDIO_SINT32;

    uint32_t bufferFrames = renderFrameCount;
    auto err = m_rtAudio->openStream(&params, nullptr, audioFmt, sampleRate,
                                     &bufferFrames, &audioCallback, this);
    if (err != RTAUDIO_NO_ERROR)
    {
        qDebug() << "打开音频设备失败";
        return;
    }
    err = m_rtAudio->startStream();
    if (err != RTAUDIO_NO_ERROR)
    {
        qDebug() << "播放音频失败";
        return;
    }

    m_sampleRate = sampleRate;
    m_bitPerSample =bitPerSample;
    m_channelCount = channelCount;
    m_renderFrameCount = renderFrameCount;
}


// 回调函数：实时填充音频数据
int AudioRender::AudioOutputCallback(void* outputBuffer, unsigned int nFrames)
{
    size_t bufferSize = nFrames * m_channelCount * m_bitPerSample / 8;
    memset(outputBuffer, 0, bufferSize);

    int16_t* bufferShort = static_cast<int16_t*>(outputBuffer);
    auto readSize = m_audioBuffer->PopData(bufferShort, bufferSize);
    m_audioBufferCV.notify_all();
    return readSize;
}

void AudioRender::Stop()
{
    if (m_audioBuffer)
        m_audioBuffer->UnInit();

    if (m_rtAudio->isStreamRunning())
    {
        auto err = m_rtAudio->stopStream();
        if (err != RTAUDIO_NO_ERROR)
        {
            qDebug() << "停止音频播放失败";
            return;
        }
    }
    if (m_rtAudio->isStreamOpen())
    {
        m_rtAudio->closeStream();
    }
}

void AudioRender::SetVolume(float volume)
{
    m_audioVolume = volume;
}

float AudioRender::GetVolume()
{
    return m_audioVolume;
}

void AudioRender::Write(const AudioFrame&frame)
{
    if(!m_audioBuffer || !m_audioBuffer->IsInit())
        return;

    std::unique_lock<std::mutex> lock(m_audioBufferMutex);
    m_audioBufferCV.wait(lock, [this, &frame]() {
        return m_audioBuffer->GetFreeSize() > frame.dataSize;
        });

    if (frame.bitPerSample == 8)
    {
        int8_t* data = reinterpret_cast<int8_t*>(frame.audioData);
        m_audioBuffer->PushData(data, frame.dataSize);
    }
    else if (frame.bitPerSample == 16)
    {
        int16_t* data = reinterpret_cast<int16_t*>(frame.audioData);
        m_audioBuffer->PushData(data, frame.dataSize);
    }
    else if (frame.bitPerSample == 32)
    {
        int32_t* data = reinterpret_cast<int32_t*>(frame.audioData);
        m_audioBuffer->PushData(data, frame.dataSize);
    }
    return;
}
