#ifndef MEDIA_DEFINE_H
#define MEDIA_DEFINE_H

#include <string>
#include <functional>
#include <opencv2/opencv.hpp>

enum class ReaderErrorType
{

};

struct ReaderError
{
    ReaderErrorType errType;
    std::string errMsg;
};
using MediaErrorCallback = std::function<void(const ReaderError&)>;

struct VideoFrame
{
    cv::Mat videoData;
};
using VideoCallback = std::function<void(const VideoFrame&)>;

struct AudioFrame
{
    void* audioData = nullptr;
    size_t dataSize = 0;
    int16_t sampleRate = 0;
    int16_t bitPerSample = 0;
    int16_t channelCount = 0;
};
using AudioCallback = std::function<void(const AudioFrame&)>;

struct MediaParameter
{
    std::string url;

    int16_t outputSampleRate = 0;
    int16_t outputBitPerSample = 0;
    int16_t outputChannelCount = 0;

    std::string hwDeviceName;

    MediaErrorCallback errCallback = nullptr;
    VideoCallback videoCallback = nullptr;
    AudioCallback audioCallback = nullptr;
};

#endif // MEDIA_DEFINE_H
