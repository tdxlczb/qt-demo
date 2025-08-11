#include "audio_render.h"
#include <QDebug>
#include "RtAudio.h"

//#define DEBUG_PCM
#ifdef DEBUG_PCM
#include <fstream>
std::ofstream g_pcmOutput;
std::ofstream g_pcmInput;
#endif

AudioRender::AudioRender(bool useJitterBuffer)
    : m_audioBuffer(new DynamicJitterBuffer())
    , m_rtAudio(new RtAudio())
    , m_isUseJitterBuffer(useJitterBuffer)
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

void AudioRender::Start(const AudioSpec& audioSpec, int renderFrameCount)
{
    if (m_isUseJitterBuffer) {
        if (!m_audioBuffer->IsInit())
        {
            if (audioSpec.bitPerSample == 8)
                m_audioBuffer->Init<int8_t>();
            else if (audioSpec.bitPerSample == 16)
                m_audioBuffer->Init<int16_t>();
            else if (audioSpec.bitPerSample == 32)
                m_audioBuffer->Init<int32_t>();
        }
    }
    RtAudio::StreamParameters params;
    params.deviceId = m_rtAudio->getDefaultOutputDevice(); // 默认输出设备
    params.nChannels = audioSpec.channels;                   // 立体声
    params.firstChannel = 0;                       // 起始声道

    RtAudioFormat audioFmt;
    if (audioSpec.bitPerSample == 8)
        audioFmt = RTAUDIO_SINT8;
    else if (audioSpec.bitPerSample == 16)
        audioFmt = RTAUDIO_SINT16;
    else if (audioSpec.bitPerSample == 32)
        audioFmt = RTAUDIO_SINT32;

    uint32_t bufferFrames = renderFrameCount;
    auto err = m_rtAudio->openStream(&params, nullptr, audioFmt, audioSpec.sampleRate,
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

    m_audioSpec = audioSpec;
    m_renderFrameCount = renderFrameCount;
}


// 回调函数：实时填充音频数据
int AudioRender::AudioOutputCallback(void* outputBuffer, unsigned int nFrames)
{
    size_t sampleCount = nFrames * m_audioSpec.channels;
    size_t bufferSize = sampleCount * m_audioSpec.bitPerSample / 8;
    if (!m_isUseJitterBuffer) {
        m_pcmCallback(reinterpret_cast<uint8_t*>(outputBuffer), bufferSize);
        return 0;
    }

    memset(outputBuffer, 0, bufferSize);

    int16_t* outputShort = static_cast<int16_t*>(outputBuffer);
    float volume = GetVolume();
    if (volume == 1.0f) {
        auto readSize = m_audioBuffer->PopData(outputShort, bufferSize);
        m_audioBufferCV.notify_all();
        return readSize;
    }

    int16_t* inputShort = new int16_t[bufferSize / sizeof(int16_t)];
    memset(inputShort, 0, bufferSize);
    auto readSize = m_audioBuffer->PopData(inputShort, bufferSize);
    for (size_t i = 0; i < sampleCount; i++)
    {
        int32_t temp = static_cast<int32_t>(inputShort[i]) * volume;
        // 饱和处理防止溢出
        if (temp > 32767)//INT16_MAX等于32767
            temp = 32767;
        else if (temp < -32768)//INT16_MIN等于-32768
            temp = -32768;
        outputShort[i] = static_cast<int16_t>(temp);
    }
    delete[] inputShort;
    inputShort = nullptr;

    m_audioBufferCV.notify_all();
    return 0;
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

void AudioRender::SetPCMCallback(PCMCallback callback)
{
    m_pcmCallback = callback;
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
        return m_audioBuffer->GetFreeSize() > frame.size;
        });

    if (frame.spec.bitPerSample == 8)
    {
        int8_t* data = reinterpret_cast<int8_t*>(frame.data);
        m_audioBuffer->PushData(data, frame.size);
    }
    else if (frame.spec.bitPerSample == 16)
    {
        int16_t* data = reinterpret_cast<int16_t*>(frame.data);
        m_audioBuffer->PushData(data, frame.size);
    }
    else if (frame.spec.bitPerSample == 32)
    {
        int32_t* data = reinterpret_cast<int32_t*>(frame.data);
        m_audioBuffer->PushData(data, frame.size);
    }
    return;
}
