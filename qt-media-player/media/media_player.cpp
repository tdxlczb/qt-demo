#include "media_player.h"
#include "log.h"
#include "media_utils.h"
#include "media_audio_filter.h"
#include "media_decoder.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

namespace mp {

/**
* 较新版本的av_packet_free、av_frame_free内部都先执行了av_packet_unref、av_frame_unref，因此没必要再多调一次
*/

MediaPlayer::MediaPlayer()
    : m_uid(uuid32())
{
    avformat_network_init();
}

MediaPlayer::~MediaPlayer()
{
    avformat_network_deinit();
}

std::string MediaPlayer::GetId() const
{
    return m_uid;
}

std::string MediaPlayer::GetTag() const
{
    return m_tag;
}

std::string MediaPlayer::GetPlayUrl() const
{
    return m_url;
}

void MediaPlayer::SetTag(const std::string& tag)
{
    m_tag = tag;
}

void MediaPlayer::SetPlayEvent(PlayEvent* playEvent)
{
    m_playEvent = playEvent;
}

void MediaPlayer::Play(const std::string& url, const PlayOptions& options)
{
    Stop();
    m_url = url;
    m_options = options;

    m_isThreadRun.store(true);
    if (!m_thDemuxer.joinable())
    {
        m_thDemuxer = std::thread(&MediaPlayer::DemuxThread, this);
    }
}

void MediaPlayer::Stop()
{
    m_url = "";
    m_options = PlayOptions{};

    m_isThreadRun.store(false);
    if (m_thVideoDisplay.joinable()) {
        m_thVideoDisplay.join();
    }
    if (m_thVideoDecoder.joinable()) {
        m_thVideoDecoder.join();
    }
    if (m_thAudioDecoder.joinable()) {
        m_thAudioDecoder.join();
    }
    if (m_thDemuxer.joinable()) {
        m_thDemuxer.join();
    }
    StreamClose();

    m_isStreamOver.store(false);
    m_videoPacketQueue.Clear();
    m_audioPacketQueue.Clear();
    m_videoFrameQueue.Clear();

    m_videoPacketIndex = 0;
    m_audioPacketIndex = 0;
    m_videoFrameIndex = 0;
    m_audioFrameIndex = 0;
    m_iLastCountTime = 0;
}

void MediaPlayer::Pause()
{
    m_pauseReq.store(true);
    //if (m_isPaused.load()) {
    //    // 恢复读取（FFmpeg 4.0+）
    //    if (m_formatContext->iformat && strcmp(m_formatContext->iformat->name, "rtsp") == 0) {
    //        int ret = av_read_play(m_formatContext);  // 内部发送RTSP PLAY
    //        m_isPaused.store(false);
    //    }
    //}
    //else {
    //    // 暂停读取（FFmpeg 4.0+）
    //    if (m_formatContext->iformat && strcmp(m_formatContext->iformat->name, "rtsp") == 0) {
    //        int ret = av_read_pause(m_formatContext); // 内部发送RTSP PAUSE
    //        m_isPaused.store(true);
    //    }
    //}
    //if(m_isPaused.load()){
    //    m_pRtspPlayer->pause(false);
    //    m_isPaused.store(false);
    //}
    //else {
    //    m_pRtspPlayer->pause(true);
    //    m_isPaused.store(true);
    //}
    //m_pRtspPlayer->seekTo((uint32_t)120000);
}

void MediaPlayer::Resume()
{
}

void MediaPlayer::Speed(double speed)
{
    m_speed = speed;
    m_videoClock.set_clock_speed(speed);
    m_audioClock.set_clock_speed(speed);
    m_extClock.set_clock_speed(speed);
    //m_pRtspPlayer->speed(speed);
    //if (m_isMediaFile) {
    //    m_speed = speed;
    //    if (m_pAudioSpeedFilter) {
    //        m_pAudioSpeedFilter->ChangeSpeed(speed);
    //    }
    //}
}

void MediaPlayer::Seek(int64_t seconds)
{
    if (m_isMediaFile) {
        m_seekPos = seconds;
        m_seekReq.store(true);
    }
}

void MediaPlayer::SeekTo(int64_t seconds)
{
}

int64_t MediaPlayer::GetDuration()
{

    return 0;
}

bool MediaPlayer::StreamOpen()
{
    return false;
}

void MediaPlayer::StreamClose()
{
    CloseDecoder();
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
}

void MediaPlayer::StreamDemux()
{
}

bool MediaPlayer::CreateVideoDecoder(AVCodecID id, AVCodecParameters* codecpar)
{
    if (!m_pVideoDecoder) {
        m_pVideoDecoder = std::make_unique<VideoDecoder>();
    }
    if (!m_pVideoDecoder->OpenDecoder(id, m_options.hwdevice, codecpar)) {
        return false;
    }
    m_pVideoDecoder->SetOnDecodeFrame(std::bind(&MediaPlayer::OnVideoFrame, this, std::placeholders::_1));
    if (!m_thVideoDecoder.joinable()) {
        m_thVideoDecoder = std::thread(&MediaPlayer::VideoThread, this);
    }
    return true;
}

bool MediaPlayer::CreateAudioDecoder(AVCodecID id, AVCodecParameters* codecpar)
{
    if (!m_pAudioDecoder) {
        m_pAudioDecoder = std::make_unique<AudioDecoder>();
    }
    if (!m_pAudioDecoder->OpenDecoder(id, codecpar)) {
        return false;
    }
    m_pAudioDecoder->SetOnDecodeFrame(std::bind(&MediaPlayer::OnAudioFrame, this, std::placeholders::_1));
    if (!m_thAudioDecoder.joinable()) {
        m_thAudioDecoder = std::thread(&MediaPlayer::AudioThread, this);
    }
    return true;
}

void MediaPlayer::CloseDecoder()
{
    if (m_pVideoDecoder)
    {
        m_pVideoDecoder->CloseDecoder();
    }
    if (m_pAudioDecoder)
    {
        m_pAudioDecoder->CloseDecoder();
    }
}

void MediaPlayer::DemuxThread()
{
    m_videoClock.init_clock();
    m_audioClock.init_clock();
    m_extClock.init_clock();

    m_isMediaFile = !IsNetworkStream(m_url);

    if (!StreamOpen()) {
        LOG_ERROR << PLAYTAG << "stream open failed";
        return;
    }

    if (m_isMediaFile) {
        m_videoPacketQueue.Resize(10);
        m_audioPacketQueue.Resize(10);
    } else {
        m_videoPacketQueue.Resize(1000);
        m_audioPacketQueue.Resize(1000);
    }

    if (m_isAsyncDisplay && !m_thVideoDisplay.joinable()) {
        m_thVideoDisplay = std::thread(&MediaPlayer::DisplayThread, this);
    }
    LOG_INFO << PLAYTAG << "play start";
    StreamDemux();
    LOG_INFO << PLAYTAG << "play end";
}

void MediaPlayer::VideoThread()
{
    while (m_isThreadRun.load())
    {
        //播放流时，流结束就退出循环
        if (m_isStreamOver.load() && !m_isMediaFile)
            break;
        //播放文件时，文件结束，并且队列为空才退出循环
        if (m_isStreamOver.load() && m_isMediaFile && m_videoPacketQueue.Size() <= 0)
            break;

        if (m_isPaused.load()) {
            av_usleep(1000);
            continue;
        }
        AVPacket* packet = m_videoPacketQueue.PopFront();
        if (!packet) {
            av_usleep(1000);//windows系统sleep精度是15ms，放到前面使用sleep会导致处理太慢
            //std::this_thread::yield(); //让出时间片，无数据时降低CPU到30-50%
            continue;
        }

        m_videoPacketIndex++;
        if (m_pVideoDecoder) {
            m_pVideoDecoder->SendPacket(packet);
        }
        av_packet_free(&packet);
    }

    m_videoPacketQueue.Clear();//线程退出时清空队列，避免队列阻塞等待卡死
}

void MediaPlayer::AudioThread()
{
    while (m_isThreadRun.load())
    {
        //播放流时，流结束就退出循环
        if (m_isStreamOver.load() && !m_isMediaFile)
            break;
        //播放文件时，文件结束，并且队列为空才退出循环
        if (m_isStreamOver.load() && m_isMediaFile && m_audioPacketQueue.Size() <= 0)
            break;

        if (m_isPaused.load()) {
            av_usleep(1000);
            continue;
        }
        AVPacket* packet = m_audioPacketQueue.PopFront();
        if (!packet) {
            av_usleep(1000);//windows系统sleep精度是15ms，放到前面使用sleep会导致处理太慢
            //std::this_thread::yield(); //让出时间片，无数据时降低CPU到30-50%
            continue;
        }

        m_audioPacketIndex++;

        if (m_speed >= 0.5 && m_speed <= 2.0) {
            if (m_pAudioDecoder) {
                m_pAudioDecoder->SendPacket(packet);
            }
        }
        else {
            int64_t npts = packet->pts == AV_NOPTS_VALUE ? 0 : packet->pts;
            double pts = npts * m_audioTimebae;
            //更新时钟
            m_audioClock.set_clock(pts);
            m_extClock.sync_clock_to_slave(m_audioClock.get_clock());
        }

        av_packet_free(&packet);
    }
    m_audioPacketQueue.Clear();//线程退出时清空队列，避免队列阻塞等待卡死
}

//static AudioPCMWriter g_filter1("audio_fileter1.pcm");
//static AudioPCMWriter g_filter2("audio_fileter2.pcm");

void MediaPlayer::OnVideoFrame(AVFrame* frame)
{
    if (frame->width <= 0 || frame->height <= 0) {//该帧不可用，舍弃
        return;
    }

    m_videoFrameIndex++;
    //LOG_INFO << PLAYTAG << "decode video frame index:" << m_videoFrameIndex << ", pts:" << frame->pts;
    int64_t curTime = av_gettime_relative();
    if (curTime - m_iLastCountTime > 4000000) {
        LOG_INFO << PLAYTAG << "frame index:" << m_videoFrameIndex << ", fps:" << m_videoFrameIndex - m_iLastCountFrameIndex;
        m_iLastCountFrameIndex = m_videoFrameIndex;
        m_iLastCountTime = curTime;
    }
    if (m_isAsyncDisplay) {
        m_videoFrameQueue.Push(frame);
    }
    else {
        DisplayVideo(frame);
    }
}

void MediaPlayer::OnAudioFrame(AVFrame* frame)
{
    //LOG_DEBUG << PLAYTAG << "decode audio frame index:" << m_audioFrameIndex;
    m_audioFrameIndex++;

    if (m_speed != 1.0) {
        if (!m_pAudioSpeedFilter) {
            m_pAudioSpeedFilter = new AudioSpeedFilter();
            m_pAudioSpeedFilter->Init(frame->sample_rate, (AVSampleFormat)frame->format, frame->channel_layout, m_speed);
        }
        // 1. 发送解码帧到滤镜
        m_pAudioSpeedFilter->SendFrame(frame);

        // 2. 循环获取倍速后的帧（可能1输入对应多输出或少输出）
        while (true) {
            AVFrame* filteredFrame = m_pAudioSpeedFilter->ReceiveFrame();
            if (!filteredFrame)
                break; // 没有更多输出
            filteredFrame->pts = filteredFrame->pkt_dts;
            //size_t bufferSize = av_samples_get_buffer_size(filteredFrame->linesize, filteredFrame->channels, filteredFrame->nb_samples, (AVSampleFormat)filteredFrame->format, 1);
            //g_filter1.Write(reinterpret_cast<const char*>(filteredFrame->data[0]), bufferSize);
            DisplayAudio(filteredFrame);

            // 4. 释放处理后的帧
            av_frame_free(&filteredFrame);
        }
    }
    else {
        DisplayAudio(frame);
    }
}

void MediaPlayer::DisplayThread()
{
    while (m_isThreadRun.load() && !m_isStreamOver.load())
    {
        AVFrame* frame = m_videoFrameQueue.PopFront();
        if (!frame)
            continue;

        DisplayVideo(frame);
        av_frame_free(&frame);
    }
}

void MediaPlayer::DisplayVideo(AVFrame* frame)
{
    int64_t npts = frame->pts == AV_NOPTS_VALUE ? 0 : frame->pts;
    double pts = npts * m_videoTimebae;
    double now = av_gettime_relative() / 1000000.0;
    //LOG_INFO << PLAYTAG << "===== video frameIndex:" << m_videoFrameIndex << ", pts:" << npts << ", ts:" << pts;
    //if (m_lastVideoPts != 0.0 && ((pts - m_lastVideoPts) / m_speed) < 0.016) {
    //    //帧率太高没有意义，太快的帧舍弃
    //    return;
    //}
    m_lastVideoPts = pts;
    double clock = m_videoClock.get_clock();
    double master = m_audioClock.get_clock();
    if (isnan(master)) {
        master = m_extClock.get_clock();
    }
    // 使用wait2如果遇到刚开始rtp包时间戳不准确的情况，可能会导致等待时间过长。
    // 例如有的时候开始的包是pts是0.16s，过了一小会恢复正常的4688.503s，如果直接使用wait2，可能会要等到最大阈值10s
    // 如果使用wait,短时间内频繁同步音频时间戳，可以规避这个问题
    // m_videoClock.wait2(pts, master, m_speed);

    if (m_speed <= 4.0) {
        //只播关键帧时，音画同步有点问题，关闭音画同步
        while (true)
        {
            if (!m_videoClock.wait(clock, master)) {
                //av_usleep(1000.0);
                continue;
            }
            else {
                break;
            }
        }
    }
    //更新时钟
    m_videoClock.set_clock(pts);
    m_extClock.sync_clock_to_slave(m_videoClock.get_clock());

    VideoFrame outFrame;
    for (size_t i = 0; i < 8; i++)
    {
        outFrame.linedata[i] = frame->data[i];
        outFrame.linesize[i] = frame->linesize[i];
    }
    outFrame.pts = pts;
    outFrame.index = m_videoFrameIndex;
    outFrame.spec.width = frame->width;
    outFrame.spec.height = frame->height;
    outFrame.spec.format = frame->format;
    if (m_playEvent) {
        m_playEvent->onVideoFrame(outFrame);
    }

}

void MediaPlayer::DisplayAudio(AVFrame* frame)
{
    int64_t npts = frame->pts == AV_NOPTS_VALUE ? 0 : frame->pts;
    auto clock = m_audioClock.get_clock();
    double pts = npts * m_audioTimebae;
    double now = av_gettime_relative() / 1000000.0;
    //LOG_INFO << PLAYTAG << "audio frameIndex:" << m_audioFrameIndex << ", pts:" << pts << ", clock:" << clock;
    while (true)
    {
        if (!m_audioClock.wait(pts, -1.0)) {
            //av_usleep(1000.0);
            continue;
        }
        else {
            break;
        }
    }
    //g_filter2.WriteFrame(frame);

    //更新时钟
    m_audioClock.set_clock(pts);
    m_extClock.sync_clock_to_slave(m_audioClock.get_clock());

    AudioFrame outFrame;
    for (size_t i = 0; i < 8; i++)
    {
        outFrame.linedata[i] = frame->data[i];
        outFrame.linesize[i] = frame->linesize[i];
    }
    outFrame.nbSamples = frame->nb_samples;
    outFrame.pts = pts;
    outFrame.index = m_audioFrameIndex;
    outFrame.spec.sampleRate = frame->sample_rate;
    outFrame.spec.channels = frame->channels;
    outFrame.spec.format = frame->format;

    if (m_playEvent) {
        m_playEvent->onAudioFrame(outFrame);
    }


}

} // namespace mp