#ifndef GB_READER_H
#define GB_READER_H

#include <thread>
#include <atomic>
#include "media_reader.h"

#include <opencv2/opencv.hpp>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

struct GBParameter : public MediaParameter
{

};

class GBReader : public MediaReader
{
public:
    GBReader(const std::string& sReaderId);
    ~GBReader();

    bool Init(std::shared_ptr<MediaParameter> pParam) override;
    bool UnInit() override;
    bool Start() override;
    bool Stop() override;

private:
    void VideoReader();
    void AudioReader();

private:
    AVFormatContext* m_formatContext = nullptr;
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    AVCodecContext* m_videoCodecContext = nullptr;
    AVCodecContext* m_audioCodecContext = nullptr;
    SwsContext* m_videoSwsContext = nullptr;//视频重采样
    SwsContext* m_audioSwsContext = nullptr;//音频重采样

    std::thread m_thVideoReader;
    std::atomic_bool m_bVideoThreadRunning = false;
    std::thread m_thAudioReader;
    std::atomic_bool m_bAudioThreadRunning = false;
};


#endif // GB_READER_H
