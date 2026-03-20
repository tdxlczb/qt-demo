#include "media_display.h"

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

#include "media_log.h"
#include "media/media_utils.h"

namespace mp {

VideoConverter::VideoConverter(const VideoSpec& dstSpec, const std::string& id)
    : m_dstSpec(dstSpec)
    , m_converterId(id)
{

}

VideoConverter::~VideoConverter()
{
    m_pCallback = nullptr;
    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
        m_pSwsCxtVideo = nullptr;
    }
    if (m_pFrameDst) {
        av_freep(&m_pFrameDst->data[0]);
        av_frame_free(&m_pFrameDst);
    }
}

VideoSpec VideoConverter::GetSrcSpec()
{
    return m_srcSpec;
}

VideoSpec VideoConverter::GetDstSpec()
{
    return m_dstSpec;
}

void VideoConverter::SetCallback(const VideoCallback& callback)
{
    m_pCallback = callback;
}

PlayError VideoConverter::InitSwsContext(const VideoSpec& srcSpec)
{
    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
    }
    LOG_INFO << "converterId:" << m_converterId << " src width height format:" << srcSpec.width << "x" << srcSpec.height << "," << srcSpec.format;
    LOG_INFO << "converterId:" << m_converterId << " dst width height format:" << m_dstSpec.width << "x" << m_dstSpec.height << "," << m_dstSpec.format;
    m_pSwsCxtVideo = sws_getContext(srcSpec.width, srcSpec.height, (AVPixelFormat)srcSpec.format,
        m_dstSpec.width, m_dstSpec.height, (AVPixelFormat)m_dstSpec.format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (nullptr == m_pSwsCxtVideo) {
        LOG_ERROR << "converterId:" << m_converterId << " m_pSwsCxtVideo create failed";
        return PlayError{ PlayErrorCode::kCreateConverterFailed,"" };
    }

    LOG_INFO << "converterId:" << m_converterId << " video sws init ok";
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoConverter::InitDstFrame()
{
    if (m_pFrameDst &&
        m_pFrameDst->format == m_dstSpec.format &&
        m_pFrameDst->width == m_dstSpec.width &&
        m_pFrameDst->height == m_dstSpec.height) {
        return PlayError{ PlayErrorCode::kNoError,"" };
    }

    if (m_pFrameDst) {
        av_freep(&m_pFrameDst->data[0]);
        av_frame_free(&m_pFrameDst);
    }

    m_pFrameDst = av_frame_alloc();
    if (nullptr == m_pFrameDst) {
        LOG_ERROR << "converterId:" << m_converterId << " av_frame_alloc failed";
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    m_pFrameDst->format = m_dstSpec.format;
    m_pFrameDst->width = m_dstSpec.width;
    m_pFrameDst->height = m_dstSpec.height;
    //av_image_alloc申请一块连续内存，pointers[i]的指针分别指向这块内存中各平面的起始位置，使用av_freep(&pointers[0])释放这块内存，只使用av_frame_free不能释放申请的内存
    int iRet = av_image_alloc(m_pFrameDst->data, m_pFrameDst->linesize, m_pFrameDst->width, m_pFrameDst->height, (AVPixelFormat)m_pFrameDst->format, 1);
    //av_frame_get_buffer给每个平面单独申请内存，使用av_frame_free释放每个平面的内存
    //int iRet = av_frame_get_buffer(m_pFrameDst, 64);
    if (iRet <= 0) {
        LOG_ERROR << "converterId:" << m_converterId << " av_image_alloc error:" << iRet;
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoConverter::Input(AVFrame* pFrame, VideoFrame& outFrame)
{
    //重采样操作锁，避免同时更改
    std::lock_guard<std::mutex> lock(m_swsMutex);
    if (m_dstSpec.width <= 0)
        m_dstSpec.width = pFrame->width;
    if (m_dstSpec.height <= 0)
        m_dstSpec.height = pFrame->height;
    if (m_dstSpec.format == (int)AV_PIX_FMT_NONE)
        m_dstSpec.format = pFrame->format;

    //m_dstSpec.width = 1168;
    //m_dstSpec.height = 657;
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

    //if (nullptr == m_pFrameDst) {
    auto err = InitDstFrame();
    if (err.code != PlayErrorCode::kNoError) {
        return err;
    }
    //}

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
            LOG_ERROR << "converterId:" << m_converterId << " sws_scale err:" << ret;
            return PlayError{ PlayErrorCode::kConverteFailed,"" };
        }
        buffer = m_pFrameDst->data[0];
        bufferSize = av_image_get_buffer_size((AVPixelFormat)m_pFrameDst->format, m_pFrameDst->width, m_pFrameDst->height, 1);
        outFrame.data = buffer;
        outFrame.size = bufferSize;

        for (size_t i = 0; i < 8; i++)
        {
            outFrame.linedata[i] = m_pFrameDst->data[i];
            outFrame.linesize[i] = m_pFrameDst->linesize[i];
        }
    }
    else {
        //将源帧的不连续内存数据拷贝到连续内存的帧，不连续内存后续不方便使用
        //av_image_copy(m_pFrameDst->data, m_pFrameDst->linesize, (const uint8_t**)pFrame->data, pFrame->linesize, (AVPixelFormat)pFrame->format, pFrame->width, pFrame->height);
        //buffer = m_pFrameDst->data[0];
        //bufferSize = av_image_get_buffer_size((AVPixelFormat)m_pFrameDst->format, m_pFrameDst->width, m_pFrameDst->height, 1);
        //outFrame.data = buffer;
        //outFrame.size = bufferSize;

        for (size_t i = 0; i < 8; i++)
        {
            outFrame.linedata[i] = pFrame->data[i];
            outFrame.linesize[i] = pFrame->linesize[i];
        }
    }

    int tryBufferSize = m_dstSpec.width * m_dstSpec.height * 1.5;
    if (bufferSize != tryBufferSize) {
        int ny = m_pFrameDst->linesize[0] * m_pFrameDst->height;
        int nu = m_pFrameDst->linesize[1] * ((m_pFrameDst->height + 2 - 1) / 2);//必须向上取整
        int nv = m_pFrameDst->linesize[2] * ((m_pFrameDst->height + 2 - 1) / 2);//必须向上取整
        //重采样后动态大小的yuv帧可能存在数据对齐问题，使用av_image_get_buffer_size等于ny + nu + nv
    }

    outFrame.spec = m_dstSpec;
    if (m_pCallback)
    {
        m_pCallback(outFrame);
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError VideoConverter::Input(const VideoFrame& inFrame, VideoFrame& outFrame)
{
    AVFrame* pFrame = av_frame_alloc();
    pFrame->width = inFrame.spec.width;
    pFrame->height = inFrame.spec.height;
    pFrame->format = inFrame.spec.format;
    for (size_t i = 0; i < 8; i++)
    {
        pFrame->data[i] = inFrame.linedata[i];
        pFrame->linesize[i] = inFrame.linesize[i];
    }
    auto ret = Input(pFrame, outFrame);
    av_frame_free(&pFrame);
    return ret;
}

PlayError VideoConverter::Input(AVFrame* pFrame)
{
    VideoFrame outFrame;
    return Input(pFrame, outFrame);
}

void VideoConverter::UpdateDstSize(int iDstWidth, int iDstHeight)
{
    LOG_INFO << "converterId:" << m_converterId << " try update converter size:" << iDstWidth << "," << iDstHeight;
    int iWidth = m_srcSpec.width;
    int iHeight = m_srcSpec.height;
    int gcd = GetGCD(iWidth, iHeight);
    int times = 0;
    if (gcd > 0) {
        int minW = iWidth / gcd;
        int minH = iHeight / gcd;
        iWidth = minW;
        iHeight = minH;
        //获取一个宽高刚好比播放宽高大，比例一致的宽高，宽高比例要是不一致，重采样出来的图片是混乱的
        while (iWidth <= iDstWidth && iHeight <= iDstHeight)
        {
            times += 8;//非4的倍数，在使用QImage加载时，会默认以4对齐取每行的字节数，非8的倍数，yuv渲染有点问题，这里强制保证宽高是8的倍数
            iWidth = minW * times;
            iHeight = minH * times;
        }
    }
    if (iWidth >= m_srcSpec.width || iHeight >= m_srcSpec.height) {
        //如果重新计算出来的宽高大于源宽高的，则不更改重采样器
        LOG_INFO << "converter size:" << iWidth << "," << iHeight << " is over origin size:" << m_srcSpec.width << "," << m_srcSpec.height;
        return;
    }
    if (m_dstSpec.width == iWidth && m_dstSpec.height == iHeight) {
        //如果重新计算出来的宽高等于目标宽高，则不更改重采样器
        LOG_INFO << "change converter size repeat";
        return;
    }

    //由于触屏电脑可能会多次快速触发大小的更改，这里不采用时间限时重采样器的更改，而是使用大小变化率来限制，大小变化率小的可以不更改重采样器
    double scaleW = std::abs(iWidth - m_dstSpec.width) / (float)m_dstSpec.width;
    double scaleH = std::abs(iHeight - m_dstSpec.height) / (float)m_dstSpec.height;
    if (scaleW < 0.2 && scaleH < 0.2) {
        LOG_INFO << "change converter size little";
        return;
    }

    int64_t curTime = av_gettime_relative();
    //避免快速更改大小导致的重新初始化重采样器，限制0.1s内不更改重采样器
    if (curTime - m_lastUpdateSizeTime < 100000) {
        LOG_INFO << "change converter size too fast";
        return;
    }

    //这里释放m_pSwsCxtVideo和m_pFrameDest触发重采样的重新初始化，加锁避免操作重叠
    std::lock_guard<std::mutex> lock(m_swsMutex);
    m_dstSpec.width = iWidth;
    m_dstSpec.height = iHeight;
    LOG_INFO << "converterId:" << m_converterId << " actually update converter size:" << iWidth << "," << iHeight;

    if (m_pSwsCxtVideo) {
        sws_freeContext(m_pSwsCxtVideo);
        m_pSwsCxtVideo = nullptr;
    }
    //不能在这里更改重采样帧的内存，否则可能会出现外部使用该内存时，刚好触发此处的内存释放，导致崩溃
    //if (m_pFrameDst) {
    //    av_freep(&m_pFrameDst->data[0]);
    //    av_frame_free(&m_pFrameDst);
    //    m_pFrameDst = nullptr;
    //}

    m_lastUpdateSizeTime = curTime;
}

AudioConverter::AudioConverter(const AudioSpec& dstSpec, const std::string& id)
    : m_dstSpec(dstSpec)
    , m_converterId(id)
{
}

AudioConverter::~AudioConverter()
{
    m_pCallback = nullptr;
    if (m_pSwrCxtAudio) {
        swr_free(&m_pSwrCxtAudio);
        m_pSwrCxtAudio = nullptr;
    }
    if (m_pFrameDst) {
        //av_freep(&m_pFrameDst->data[0]);
        av_frame_free(&m_pFrameDst);
    }
}

AudioSpec AudioConverter::GetSrcSpec()
{
    return m_srcSpec;
}

AudioSpec AudioConverter::GetDstSpec()
{
    return m_dstSpec;
}

void AudioConverter::SetCallback(const AudioCallback& callback)
{
    m_pCallback = callback;
}

PlayError AudioConverter::InitSwrContext(const AudioSpec& srcSpec)
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
        LOG_ERROR << "converterId:" << m_converterId << " m_pSwrCxtAudio create failed";
        LOG_ERROR << "converterId:" << m_converterId << " src sample_rate:" << srcSpec.sampleRate;
        LOG_ERROR << "converterId:" << m_converterId << " src channels:" << srcSpec.channels;
        LOG_ERROR << "converterId:" << m_converterId << " src sample_fmt:" << srcSpec.format;
        LOG_ERROR << "converterId:" << m_converterId << " dst sample_rate:" << m_dstSpec.sampleRate;
        LOG_ERROR << "converterId:" << m_converterId << " dst channels:" << m_dstSpec.channels;
        LOG_ERROR << "converterId:" << m_converterId << " dst sample_fmt:" << m_dstSpec.format;
        return PlayError{ PlayErrorCode::kCreateConverterFailed,"" };
    }
    LOG_INFO << "converterId:" << m_converterId << " audio swr init ok";
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError AudioConverter::InitDstFrame()
{
    if (m_pFrameDst) {
        //av_freep(&m_pFrameDst->data[0]);
        av_frame_free(&m_pFrameDst);
    }

    m_pFrameDst = av_frame_alloc();
    if (nullptr == m_pFrameDst) {
        LOG_ERROR << "converterId:" << m_converterId << " av_frame_alloc failed";
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    m_pFrameDst->sample_rate = m_dstSpec.sampleRate;
    m_pFrameDst->channel_layout = av_get_default_channel_layout(m_dstSpec.channels);
    m_pFrameDst->channels = m_dstSpec.channels;
    m_pFrameDst->format = m_dstSpec.format;
    //m_pFrameDst->nb_samples = 1024; //这里必须设置值，否则av_samples_alloc申请不到内存 AVFrame内部nb_samples默认设置为1024
    //有可能遇到有些流将多个音频packet组成一个packet，导致输入nb_samples非常大，输出nb_samples=1024不够用
    m_pFrameDst->nb_samples = 1024 * 1024; // 需要申请更大的内存，不能使用av_samples_alloc，只能使用外部申请内存，或者av_frame_get_buffer

    //av_samples_alloc申请一块连续内存，pointers[i]的指针分别指向这块内存中各平面的起始位置，使用av_freep(&pointers[0])释放这块内存，只使用av_frame_free不能释放申请的内存
    //int nSize = av_samples_alloc(m_pFrameDst->data, m_pFrameDst->linesize, m_pFrameDst->channels, m_pFrameDst->nb_samples, (AVSampleFormat)m_pFrameDst->format, 1);
    //av_frame_get_buffer给每个平面单独申请内存，使用av_frame_free释放每个平面的内存，不需要再使用av_freep
    int iRet = av_frame_get_buffer(m_pFrameDst, 64);
    if (AVERROR(iRet)) {
    //if (nSize <= 0) {
        LOG_ERROR << "converterId:" << m_converterId << " av_frame_get_buffer error:" << iRet;
        return PlayError{ PlayErrorCode::kOutOfMemory,"" };
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

//#define DEBUG_PCM
#ifdef DEBUG_PCM
#include <fstream>
static std::ofstream g_pcmOutput;
static std::ofstream g_pcmInput;
#endif

PlayError AudioConverter::Input(AVFrame* pFrame, AudioFrame& outFrame)
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
#ifdef DEBUG_PCM
        g_pcmOutput.open("audio_convert_output.pcm", std::ios::binary);
        if (!g_pcmOutput.is_open())
        {
            qDebug() << "open failed";
        }

        g_pcmInput.open("audio_convert_input.pcm", std::ios::binary);
        if (!g_pcmInput.is_open())
        {
            qDebug() << "open failed";
        }
#endif // DEBUG_PCM
    }
    if (nullptr == m_pFrameDst || needInitSwrContext) {
        auto err = InitDstFrame();
        if (err.code != PlayErrorCode::kNoError) {
            return err;
        }
    }
#ifdef DEBUG_PCM
    if (g_pcmInput.is_open())
    {
        size_t bufferSize = av_samples_get_buffer_size(pFrame->linesize, pFrame->channels, pFrame->nb_samples, (AVSampleFormat)pFrame->format, 1);
        g_pcmInput.write(reinterpret_cast<const char*>(pFrame->data[0]), bufferSize);
        g_pcmInput.flush();
    }
#endif // DEBUG_PCM
    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;
    if (needSwrConvert) {
        int max_out_nb_samples = av_rescale_rnd(pFrame->nb_samples, m_dstSpec.sampleRate, pFrame->sample_rate, AV_ROUND_UP);
        m_pFrameDst->nb_samples = max_out_nb_samples;
        //int ret = swr_convert_frame(m_pSwrCxtAudio, m_pFrameDst, pFrame);
        int out_nb_samples = swr_convert(m_pSwrCxtAudio, m_pFrameDst->data, max_out_nb_samples, (const uint8_t**)pFrame->data, pFrame->nb_samples);
        if (out_nb_samples < 0) {
            LOG_ERROR << "converterId:" << m_converterId << " swr_convert err," << out_nb_samples;
            return PlayError{ PlayErrorCode::kConverteFailed,"" };
        }
        buffer = m_pFrameDst->data[0];
        bufferSize = out_nb_samples * m_dstSpec.channels * av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format);
    }
    else {
        //将源帧的不连续内存数据拷贝到连续内存的帧，不连续内存后续不方便使用
        av_samples_copy(m_pFrameDst->data, pFrame->data, 0, 0, pFrame->nb_samples, pFrame->channels, (AVSampleFormat)pFrame->format);
        buffer = m_pFrameDst->data[0];
        bufferSize = m_pFrameDst->nb_samples * m_dstSpec.channels * av_get_bytes_per_sample((AVSampleFormat)m_dstSpec.format);

        //buffer = pFrame->data[0];
        //bufferSize = av_samples_get_buffer_size(pFrame->linesize, pFrame->channels, pFrame->nb_samples, (AVSampleFormat)pFrame->format, 1);
    }
#ifdef DEBUG_PCM
    if (g_pcmOutput.is_open())
    {
        g_pcmOutput.write(reinterpret_cast<const char*>(buffer), bufferSize);
        g_pcmOutput.flush();
    }
#endif // DEBUG_PCM
    outFrame.data = buffer;
    outFrame.size = bufferSize;
    outFrame.spec = m_dstSpec;
    if (m_pCallback)
    {
        m_pCallback(outFrame);
    }
    return PlayError{ PlayErrorCode::kNoError,"" };
}

PlayError AudioConverter::Input(const AudioFrame& inFrame, AudioFrame& outFrame)
{
    AVFrame* pFrame = av_frame_alloc();
    pFrame->sample_rate = inFrame.spec.sampleRate;
    pFrame->channel_layout = av_get_default_channel_layout(inFrame.spec.channels);
    pFrame->channels = inFrame.spec.channels;
    av_channel_layout_default(&pFrame->ch_layout, inFrame.spec.channels);
    pFrame->format = inFrame.spec.format;
    pFrame->nb_samples = inFrame.nbSamples;
    for (size_t i = 0; i < 8; i++)
    {
        pFrame->data[i] = inFrame.linedata[i];
        pFrame->linesize[i] = inFrame.linesize[i];
    }
    auto ret = Input(pFrame, outFrame);
    av_frame_free(&pFrame);
    return ret;
}

PlayError AudioConverter::Input(AVFrame* pFrame)
{
    AudioFrame outFrame;
    return Input(pFrame, outFrame);
}

PlayError AudioConverter::GetEndFrame(AudioFrame& outFrame)
{
    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;
    if (m_pSwrCxtAudio && m_pFrameDst) {
        int max_out_nb_samples = 2048;
        m_pFrameDst->nb_samples = max_out_nb_samples;
        int ret = swr_convert_frame(m_pSwrCxtAudio, m_pFrameDst, nullptr);
        if (ret < 0) {
            LOG_ERROR << "converterId:" << m_converterId << " swr_convert err," << ret;
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

} // namespace mp