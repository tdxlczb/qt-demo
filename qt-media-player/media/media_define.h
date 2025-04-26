#ifndef MEDIA_DEFINE_H
#define MEDIA_DEFINE_H

#include <functional>

struct AudioFrame
{
    void* audioData = nullptr;
    size_t dataSize = 0;
    int16_t sampleRate = 0;
    int16_t bitPerSample = 0;
    int16_t channelCount = 0;
};


#endif // MEDIA_DEFINE_H
