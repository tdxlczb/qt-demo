#include "media_player.h"
#include "media_log.h"
#include "media_utils.h"
#include "media_audio_filter.h"
#include "media_decoder.h"
#include "ffmpeg_demuxer.h"
#include "zlm_demuxer.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

namespace mp {

/**
* 较新版本的av_packet_free、av_frame_free内部都先执行了av_packet_unref、av_frame_unref，因此没必要再多调一次
*/

MediaPlayer::MediaPlayer(const std::string& context)
    : MediaContext(context)
{
    avformat_network_init();
}

MediaPlayer::~MediaPlayer()
{
    avformat_network_deinit();
}

void MediaPlayer::OnDemuxStatus(DemuxStatus status)
{
    if (status == DemuxStatus::StreamOver) {
        m_isStreamOver = true;
        if (m_playEvent)
            m_playEvent->onClose(PlayError());
    }
}

bool MediaPlayer::OnStream(const StreamInfo& info, AVCodecParameters* codecpar)
{
    bool result = false;
    if (info.mediaType == AVMEDIA_TYPE_VIDEO) {
        m_videoTimebae = info.GetTimebase();
        result = CreateVideoDecoder(codecpar);
    } else if (info.mediaType == AVMEDIA_TYPE_AUDIO) {
        m_audioTimebae = info.GetTimebase();
        result = CreateAudioDecoder(codecpar);
    }
    return result;
}

void MediaPlayer::OnPacket(AVPacket* pPkt)
{
    //是否是坏帧
    if (pPkt->flags & AV_PKT_FLAG_CORRUPT) {
        LOG_ERROR_T << "ignore AV_PKT_FLAG_CORRUPT";
        av_packet_unref(pPkt);
        return;
    }


    bool isKey = pPkt->flags & AV_PKT_FLAG_KEY;
    if (pPkt->stream_index == AVMEDIA_TYPE_VIDEO) {

        //static int64_t packetIndex = 0;
        //static int64_t packetCount = 0;
        //static int64_t lastTime = 0;
        //packetIndex++;
        //packetCount++;
        //int64_t now = av_gettime_relative();
        //if (now - lastTime >= 1000000) {
        //    double ts = pPkt->pts * m_videoTimebae;
        //    LOG_INFO_T << "video packetIndex:" << packetIndex << ", packetCount:" << packetCount << ", ts:" << ts;
        //    packetCount = 0;
        //    lastTime = now;
        //}

        m_videoPacketSize += pPkt->size;
        m_videoPacketQueue.Push(pPkt, true);
    } else if (pPkt->stream_index == AVMEDIA_TYPE_AUDIO) {
        m_audioPacketSize += pPkt->size;
        m_audioPacketQueue.Push(pPkt, true);
    }
}

void MediaPlayer::OnPlayError(const PlayError& error)
{}

std::string MediaPlayer::GetPlayUrl() const
{
    return m_url;
}

void MediaPlayer::SetPlayEvent(PlayEvent* playEvent)
{
    m_playEvent = playEvent;
}

void MediaPlayer::ResetClock()
{
    m_videoClock.InitClock();
    m_audioClock.InitClock();
    m_extClock.InitClock();
}

void MediaPlayer::WaitClock(double pts, bool isVideo)
{
    // 遇到一种特殊情况出现时钟异常，例如从4倍速变成8倍速，一会后再回到4倍速，时间同步出现异常
    // 解码器使用8个线程时，会缓存8个packet才会解出一个frame
    // 当开启8倍速时，音频时钟由于解码非常快，会立刻更新最新一帧的pts，但是视频时钟更新的pts会是8个packet之前的pts
    // 8倍速的音频时钟会和8个packet之前的视频时钟差距越来越大，如果这个流关键帧间隔为2s，8个packet就导致差距16s左右
    // 当大于时钟设置的最大帧播放时间(10s)时，就会导致时钟同步出现异常，最简单的做法就是将最大帧播放时间设置为20s，但是这种方法治标不治本，后续可能需要考虑优化
    if (isVideo) {
        double clock = m_videoClock.GetClock();
        double master = m_audioClock.GetClock();
        if (isnan(master)) {
            master = m_extClock.GetClock();
        }
        // 使用Wait2如果遇到刚开始rtp包时间戳不准确的情况，可能会导致等待时间过长。
        // 例如有的时候开始的包是pts是0.16s，过了一小会恢复正常的4688.503s，如果直接使用Wait2，可能会要等到最大阈值10s
        // 如果使用Wait,短时间内频繁同步音频时间戳，可以规避这个问题
        // m_videoClock.Wait2(pts, master, m_speed);

        if (m_speed <= 4.0) {
            //只播关键帧时，音画同步有点问题，关闭音画同步
            while (true)
            {
                if (!m_videoClock.Wait(clock, master)) {
                    //av_usleep(1000.0);
                    continue;
                } else {
                    break;
                }
            }
        }
        //更新时钟
        m_videoClock.SetClock(pts);
        m_extClock.SyncClockToSlave(m_videoClock.GetClock());
    } else {
        //while (true)
        //{
        //    if (!m_audioClock.Wait(pts, -1.0)) {
        //        //av_usleep(1000.0);
        //        continue;
        //    } else {
        //        break;
        //    }
        //}
        //更新时钟
        m_audioClock.SetClock(pts);
        m_extClock.SyncClockToSlave(m_audioClock.GetClock());
    }
}

void MediaPlayer::Play(const std::string& url, const PlayOptions& options)
{
    LOG_INFO_T << "try play url:" << url;

    Stop();
    m_url = url;
    m_options = options;

    if (options.demuxerId == 0) {
        m_demuxer = std::make_shared<FFmpegDemuxer>(GetContext());
    } else if (options.demuxerId == 1) {
        m_demuxer = std::make_shared<ZlmDemuxer>(GetContext());
    }

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
    if (m_demuxer)
        m_demuxer->Pause();

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
    if (m_demuxer)
        m_demuxer->Resume();
}

void MediaPlayer::Speed(double speed)
{
    m_speed = speed;
    m_videoClock.SetClockSpeed(speed);
    m_audioClock.SetClockSpeed(speed);
    m_extClock.SetClockSpeed(speed);

    if (m_demuxer)
        m_demuxer->Speed(speed);

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
    if (m_demuxer)
        m_demuxer->Seek(seconds);
}

void MediaPlayer::SeekTo(int64_t seconds)
{}

int64_t MediaPlayer::GetDuration()
{
    if (m_demuxer)
        m_demuxer->GetDuration();
    return 0;
}

bool MediaPlayer::StreamOpen()
{
    if (m_demuxer)
        return m_demuxer->Open(m_url, m_options, this);
    return false;
}

void MediaPlayer::StreamClose()
{
    if (m_demuxer)
        m_demuxer->Close();

    CloseDecoder();
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
}

void MediaPlayer::StreamDemux()
{
    if (m_demuxer)
        m_demuxer->Start();
}

bool MediaPlayer::CreateVideoDecoder(AVCodecParameters* codecpar)
{
    if (!m_pVideoDecoder) {
        m_pVideoDecoder = std::make_unique<VideoDecoder>(GetContext());
    }
    if (!m_pVideoDecoder->OpenDecoder(m_options, codecpar)) {
        return false;
    }
    m_pVideoDecoder->SetOnDecodeFrame(std::bind(&MediaPlayer::OnVideoFrame, this, std::placeholders::_1));
    if (!m_thVideoDecoder.joinable()) {
        m_thVideoDecoder = std::thread(&MediaPlayer::VideoThread, this);
    }
    return true;
}

bool MediaPlayer::CreateAudioDecoder(AVCodecParameters* codecpar)
{
    if (!m_pAudioDecoder) {
        m_pAudioDecoder = std::make_unique<AudioDecoder>(GetContext());
    }
    if (!m_pAudioDecoder->OpenDecoder(m_options, codecpar)) {
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
    ResetClock();

    m_isMediaFile = !IsNetworkStream(m_url);

    if (!StreamOpen()) {
        LOG_ERROR_T << "stream open failed";
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
    LOG_INFO_T << "play start";
    StreamDemux();
    LOG_INFO_T << "play end";
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

        if (m_speed > 4.0 && !(packet->flags & AV_PKT_FLAG_KEY)) {
            av_packet_free(&packet);
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

        //if (m_speed >= 0.5 && m_speed <= 2.0) {
        if (m_pAudioDecoder) {
            m_pAudioDecoder->SendPacket(packet);
        }
        //}
        //else {
        //    int64_t npts = packet->pts == AV_NOPTS_VALUE ? 0 : packet->pts;
        //    double pts = npts * m_audioTimebae;
        //    //更新时钟
        //    m_audioClock.SetClock(pts);
        //    m_extClock.SyncClockToSlave(m_audioClock.GetClock());
        //}

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
    //LOG_INFO_T << "decode video frame index:" << m_videoFrameIndex << ", pts:" << frame->pts;
    int64_t curTime = av_gettime_relative();
    if (curTime - m_iLastCountTime > 4000000) {
        //LOG_INFO_T << "frame index:" << m_videoFrameIndex << ", fps:" << m_videoFrameIndex - m_iLastCountFrameIndex;
        m_iLastCountFrameIndex = m_videoFrameIndex;
        m_iLastCountTime = curTime;
    }
    if (m_isAsyncDisplay) {
        m_videoFrameQueue.Push(frame);
    } else {
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
    } else {
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
    //LOG_INFO_T << "===== video frameIndex:" << m_videoFrameIndex << ", pts:" << npts << ", ts:" << pts;
    //if (m_lastVideoPts != 0.0 && ((pts - m_lastVideoPts) / m_speed) < 0.016) {
    //    //帧率太高没有意义，太快的帧舍弃
    //    return;
    //}
    m_lastVideoPts = pts;

    //static int packetCount = 0;
    //packetCount++;
    //static int64_t lastTime = 0;
    //auto nowts = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //if (nowts - lastTime >= 1000000) {
    //    qInfo() << "video packet count:" << packetCount << ", last pts:" << pts << ", clock:" << m_videoClock.GetClock() << ", master:" << m_audioClock.GetClock();
    //    packetCount = 0;
    //    lastTime = nowts;
    //}

    WaitClock(pts, true);

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
    double pts = npts * m_audioTimebae;
    double now = av_gettime_relative() / 1000000.0;
    //LOG_INFO_T << "audio frameIndex:" << m_audioFrameIndex << ", pts:" << pts;
    //g_filter2.WriteFrame(frame);

    //static int packetCount = 0;
    //packetCount++;
    //static int64_t lastTime = 0;
    //auto nowts = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //if (nowts - lastTime >= 1000000) {
    //    qInfo() << "========= autio packet count:" << packetCount << ", last pts:" << pts << ", clock:" << m_audioClock.GetClock();
    //    packetCount = 0;
    //    lastTime = nowts;
    //}

    WaitClock(pts, false);

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

    if (m_playEvent && m_speed == 1.0) {
        m_playEvent->onAudioFrame(outFrame);
    }


}

} // namespace mp