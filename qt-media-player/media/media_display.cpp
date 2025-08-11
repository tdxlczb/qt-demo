#include "media_display.h"
#include <qDebug>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

//获取最大公约数
static int GetGCD(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

VideoDisplay::VideoDisplay(int displayId, const VideoSpec& dstSpec)
    : m_displayId(displayId)
    , m_dstSpec(dstSpec)
{

}

VideoDisplay::~VideoDisplay()
{
    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
        m_pSwsCxtVideo = nullptr;
    }
    if (m_pFrameDst) {
        av_freep(&m_pFrameDst[0]);
        av_frame_unref(m_pFrameDst);
        av_frame_free(&m_pFrameDst);
    }
}

void VideoDisplay::SetCallback(const VideoCallback& callback)
{
    m_pCallback = callback;
}

PlayError VideoDisplay::InitSwsContext(const VideoSpec& srcSpec)
{
    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
    }
    m_pSwsCxtVideo = sws_getContext(srcSpec.width, srcSpec.height, (AVPixelFormat)srcSpec.format,
        m_dstSpec.width, m_dstSpec.height, (AVPixelFormat)m_dstSpec.format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (nullptr == m_pSwsCxtVideo) {
        qCritical() << "DisplayId:" << m_displayId << "m_pSwsCxtVideo create failed";
        qCritical() << "DisplayId:" << m_displayId << "src width:" << srcSpec.width;
        qCritical() << "DisplayId:" << m_displayId << "src height:" << srcSpec.height;
        qCritical() << "DisplayId:" << m_displayId << "src format:" << srcSpec.format;
        qCritical() << "DisplayId:" << m_displayId << "dst width:" << m_dstSpec.width;
        qCritical() << "DisplayId:" << m_displayId << "dst height:" << m_dstSpec.height;
        qCritical() << "DisplayId:" << m_displayId << "dst format:" << m_dstSpec.format;
        return PlayError{ PlayErrorCode::kCreateConverterFailed,"" };
    }

    qInfo() << "DisplayId:" << m_displayId << "video sws init ok";
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoDisplay::InitDstFrame()
{
    if (m_pFrameDst) {
        av_freep(&m_pFrameDst[0]);
        av_frame_unref(m_pFrameDst);
        av_frame_free(&m_pFrameDst);
    }

    m_pFrameDst = av_frame_alloc();
    if (nullptr == m_pFrameDst) {
        qCritical() << "DisplayId:" << m_displayId << "av_frame_alloc failed";
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    m_pFrameDst->format = m_dstSpec.format;
    m_pFrameDst->width = m_dstSpec.width;
    m_pFrameDst->height = m_dstSpec.height;
    int iRet = av_image_alloc(m_pFrameDst->data, m_pFrameDst->linesize, m_pFrameDst->width, m_pFrameDst->height, (AVPixelFormat)m_pFrameDst->format, 1);
    if (iRet <= 0) {
        qCritical() << "DisplayId:" << m_displayId << "av_image_alloc error:" << iRet;
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoDisplay::DisplayInput(AVFrame* pFrame, VideoFrame& outFrame)
{
    //重采样操作锁，避免同时更改
    std::lock_guard<std::mutex> lock(m_swsMutex);
    if (m_dstSpec.width <= 0)
        m_dstSpec.width = pFrame->width;
    if (m_dstSpec.height <= 0)
        m_dstSpec.height = pFrame->height;
    if (m_dstSpec.format == (int)AV_PIX_FMT_NONE)
        m_dstSpec.format = pFrame->format;

    bool needInitSwsContext = false;
    if (pFrame->width != m_srcSpec.width || pFrame->height != m_srcSpec.height || pFrame->format != m_srcSpec.format) {
        m_srcSpec.width = pFrame->width;
        m_srcSpec.height = pFrame->height;
        m_srcSpec.format = pFrame->format;
        needInitSwsContext = true;
    }
    bool needSwsScale = false;
    if (pFrame->width != m_dstSpec.width || pFrame->height != m_dstSpec.height || pFrame->format != m_dstSpec.format) {
        needSwsScale = true;
    }

    if (nullptr == m_pFrameDst) {
        auto err = InitDstFrame();
        if (err.code != PlayErrorCode::kNoError) {
            return err;
        }
    }

    if (nullptr == m_pSwsCxtVideo || needInitSwsContext) {
        auto err = InitSwsContext(m_srcSpec);
        if (err.code != PlayErrorCode::kNoError) {
            return err;
        }
    }

    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;
    if (needSwsScale) {
        int ret = sws_scale(m_pSwsCxtVideo, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pFrame->height
            , m_pFrameDst->data, m_pFrameDst->linesize);
        if (ret <= 0) {
            qDebug() << "DisplayId:" << m_displayId << "sws_scale err:" << QString("%1").arg(ret);
            return PlayError{ PlayErrorCode::kConverteFailed,"" };
        }
    }
    else {
        //将源帧的不连续内存数据拷贝到连续内存的帧，不连续内存后续不方便使用
        av_image_copy(m_pFrameDst->data, m_pFrameDst->linesize, (const uint8_t**)pFrame->data, pFrame->linesize, (AVPixelFormat)pFrame->format, pFrame->width, pFrame->height);
    }
    buffer = m_pFrameDst->data[0];
    bufferSize = av_image_get_buffer_size((AVPixelFormat)m_pFrameDst->format, m_pFrameDst->width, m_pFrameDst->height, 1);

    outFrame.data = buffer;
    outFrame.size = bufferSize;
    outFrame.spec = m_dstSpec;
    if (m_pCallback)
    {
        m_pCallback(outFrame);
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoDisplay::DisplayInput(AVFrame* pFrame)
{
    VideoFrame outFrame;
    return DisplayInput(pFrame, outFrame);
}

void VideoDisplay::UpdateDisplaySize(int iDisplayWidth, int iDisplayHeight)
{
    return;
    qDebug() << "update display size:" << iDisplayWidth << "," << iDisplayHeight;
    if (m_dstSpec.width == iDisplayWidth && m_dstSpec.height)
        return;
    int64_t curTime = av_gettime_relative();
    //避免快速更改大小导致的重新初始化重采样器，限制1s内不更改重采样器
    if (curTime - m_lastUpdateSizeTime < 1000000)
        return;
    m_lastUpdateSizeTime = curTime;

    int iWidth = m_dstSpec.width;
    int iHeight = m_dstSpec.height;
    int gcd = GetGCD(iWidth, iHeight);
    int times = 1;
    if (gcd > 0) {
        int minW = iWidth / gcd;
        int minH = iHeight / gcd;
        iWidth = minW;
        iHeight = minH;
        //获取一个宽高刚好比播放宽高大，比例一致的宽高，宽高比例要是不一致，重采样出来的图片是混乱的
        while (iWidth <= iDisplayWidth && iHeight <= iDisplayHeight)
        {
            times++;
            iWidth = minW * times;
            iHeight = minH * times;
        }
    }
    if (iWidth >= m_srcSpec.width || iHeight >= m_srcSpec.height) {
        //如果重新计算出来的宽高大于源宽高的，则不更改重采样器
        return;
    }
    //这里释放m_pSwsCxtVideo和m_pFrameDest触发重采样的重新初始化，加锁避免操作重叠
    std::lock_guard<std::mutex> lock(m_swsMutex);
    m_dstSpec.width = iWidth;
    m_dstSpec.height = iHeight;
    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
        m_pSwsCxtVideo = nullptr;
    }
    if (m_pFrameDst) {
        av_freep(&m_pFrameDst[0]);
        av_frame_unref(m_pFrameDst);
        av_frame_free(&m_pFrameDst);
        m_pFrameDst = nullptr;
    }
}


AudioDisplay::AudioDisplay(int displayId, const AudioSpec& dstSpec)
    : m_displayId(displayId)
    , m_dstSpec(dstSpec)
{

}

AudioDisplay::~AudioDisplay()
{
    if (m_pSwrCxtAudio) {
        swr_free(&m_pSwrCxtAudio);
        m_pSwrCxtAudio = nullptr;
    }
    if (m_pFrameDst) {
        av_frame_unref(m_pFrameDst);
        av_frame_free(&m_pFrameDst);
    }
}

void AudioDisplay::SetCallback(const AudioCallback& callback)
{
    m_pCallback = callback;
}

PlayError AudioDisplay::InitSwrContext(const AudioSpec& srcSpec)
{
    if (m_pSwrCxtAudio) {
        swr_free(&m_pSwrCxtAudio);
    }

    //channel_layout -> channels: av_get_channel_layout_nb_channels
    //channels -> channel_layout: av_get_default_channel_layout

    m_pSwrCxtAudio = swr_alloc_set_opts(nullptr,
        av_get_default_channel_layout(m_dstSpec.channels), (AVSampleFormat)m_dstSpec.format, m_dstSpec.sampleRate,
        av_get_default_channel_layout(srcSpec.channels), (AVSampleFormat)(srcSpec.format), srcSpec.sampleRate,
        0, NULL);
    int ret = swr_init(m_pSwrCxtAudio);
    if (nullptr == m_pSwrCxtAudio || AVERROR(ret)) {
        qCritical() << "DisplayId:" << m_displayId << "m_pSwrCxtAudio create failed";
        qCritical() << "DisplayId:" << m_displayId << "src sample_rate:" << srcSpec.sampleRate;
        qCritical() << "DisplayId:" << m_displayId << "src channels:" << srcSpec.channels;
        qCritical() << "DisplayId:" << m_displayId << "src sample_fmt:" << srcSpec.format;
        qCritical() << "DisplayId:" << m_displayId << "dst sample_rate:" << m_dstSpec.sampleRate;
        qCritical() << "DisplayId:" << m_displayId << "dst channels:" << m_dstSpec.channels;
        qCritical() << "DisplayId:" << m_displayId << "dst sample_fmt:" << m_dstSpec.format;
        return PlayError{ PlayErrorCode::kCreateConverterFailed,"" };
    }
    qInfo() << "DisplayId:" << m_displayId << "audio swr init ok";
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError AudioDisplay::InitDstFrame()
{
    if (m_pFrameDst) {
        av_freep(&m_pFrameDst[0]);
        av_frame_unref(m_pFrameDst);
        av_frame_free(&m_pFrameDst);
    }

    m_pFrameDst = av_frame_alloc();
    if (nullptr == m_pFrameDst) {
        qCritical() << "DisplayId:" << m_displayId << "av_frame_alloc failed";
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    m_pFrameDst->sample_rate = m_dstSpec.sampleRate;
    m_pFrameDst->channel_layout = av_get_default_channel_layout(m_dstSpec.channels);
    m_pFrameDst->channels = m_dstSpec.channels;
    m_pFrameDst->format = m_dstSpec.format;
    m_pFrameDst->nb_samples = 1024;
    int iRet = av_frame_get_buffer(m_pFrameDst, 64);
    if (AVERROR(iRet)) {
        qCritical() << "DisplayId:" << m_displayId << "av_frame_get_buffer error:" << iRet;
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError AudioDisplay::DisplayInput(AVFrame* pFrame, AudioFrame& outFrame)
{
    if (m_dstSpec.sampleRate <= 0)
        m_dstSpec.sampleRate = pFrame->sample_rate;
    if (m_dstSpec.channels <= 0)
        m_dstSpec.channels = pFrame->channels;
    if (m_dstSpec.format == AV_SAMPLE_FMT_NONE) {
        m_dstSpec.format = pFrame->format;
        m_dstSpec.bitPerSample = av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format) * 8;
    }

    bool needInitSwrContext = false;
    if (pFrame->sample_rate != m_srcSpec.sampleRate || pFrame->channels != m_srcSpec.channels || pFrame->format != m_srcSpec.format) {
        m_srcSpec.sampleRate = pFrame->sample_rate;
        m_srcSpec.channels = pFrame->channels;
        m_srcSpec.bitPerSample = av_get_bytes_per_sample((AVSampleFormat)pFrame->format) * 8;
        m_srcSpec.format = pFrame->format;
        needInitSwrContext = true;
    }
    bool needSwrConvert = false;
    if (pFrame->sample_rate != m_dstSpec.sampleRate || pFrame->channels != m_dstSpec.channels || pFrame->format != m_dstSpec.format) {
        needSwrConvert = true;
    }

    if (nullptr == m_pSwrCxtAudio || needInitSwrContext) {
        auto err = InitSwrContext(m_srcSpec);
        if (err.code != PlayErrorCode::kNoError) {
            return err;
        }
    }
    if (nullptr == m_pFrameDst || needInitSwrContext) {
        auto err = InitDstFrame();
        if (err.code != PlayErrorCode::kNoError) {
            return err;
        }
    }

    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;
    if (needSwrConvert) {
        int max_out_nb_samples = av_rescale_rnd(pFrame->nb_samples, m_dstSpec.sampleRate, pFrame->sample_rate, AV_ROUND_UP);
        m_pFrameDst->nb_samples = max_out_nb_samples;
        int ret = swr_convert_frame(m_pSwrCxtAudio, m_pFrameDst, pFrame);
        if (ret < 0) {
            qDebug() << "DisplayId:" << m_displayId << "swr_convert err," << QString("%1").arg(ret);
            return PlayError{ PlayErrorCode::kConverteFailed,"" };
        }
        buffer = m_pFrameDst->data[0];
        bufferSize = m_pFrameDst->nb_samples * m_dstSpec.channels * av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format);
    }
    else {
        buffer = pFrame->data[0];
        bufferSize = pFrame->nb_samples * m_dstSpec.channels * av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format);
    }

    outFrame.data = buffer;
    outFrame.size = bufferSize;
    outFrame.spec = m_dstSpec;
    if (m_pCallback)
    {
        m_pCallback(outFrame);
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError AudioDisplay::DisplayInput(AVFrame* pFrame)
{
    AudioFrame outFrame;
    return DisplayInput(pFrame, outFrame);
}

PlayError AudioDisplay::GetDisplayEndFrame(AudioFrame& outFrame)
{
    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;
    if (m_pSwrCxtAudio && m_pFrameDst) {
        int max_out_nb_samples = 2048;
        m_pFrameDst->nb_samples = max_out_nb_samples;
        int ret = swr_convert_frame(m_pSwrCxtAudio, m_pFrameDst, nullptr);
        if (ret < 0) {
            qDebug() << "DisplayId:" << m_displayId << "swr_convert err," << QString("%1").arg(ret);
            return PlayError{ PlayErrorCode::kConverteFailed,"" };
        }
        buffer = m_pFrameDst->data[0];
        bufferSize = m_pFrameDst->nb_samples * m_dstSpec.channels * av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format);
    }
    outFrame.data = buffer;
    outFrame.size = bufferSize;
    outFrame.spec = m_dstSpec;
    if (m_pCallback)
    {
        m_pCallback(outFrame);
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}
