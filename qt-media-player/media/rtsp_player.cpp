#include "rtsp_player.h"
#include "media_decoder.h"

#ifdef USE_ORIGIN_ZLM

#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\jsoncpp.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\flv.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\mov.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\mpeg.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\build\3rdpart\ZLToolKit\lib\Release\ZLToolKit.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\zlmediakit.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\ext-codec.lib)")
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\srt.lib)")

#include "Rtsp/RtspDemuxer.h"
#include "Rtsp/RtspPlayer.h"

class RtspPlayerImpl
    : public mediakit::PlayerImp<mediakit::RtspPlayer, mediakit::PlayerBase>
    , private mediakit::TrackListener {
public:
    using Ptr = std::shared_ptr<RtspPlayerImpl>;
    using Super = PlayerImp<RtspPlayer, PlayerBase>;

    RtspPlayerImpl(const toolkit::EventPoller::Ptr& poller)
        : Super(poller) {
    }

    ~RtspPlayerImpl() override {}

    void Stop() { teardown(); };
    void Pause() { pause(true); };
    void Resume() { pause(false); };
    void Speed(double s) { speed(s); };
    void Seek(int64_t seconds) { seekTo((uint32_t)seconds); };
    int64_t GetDuration() { return getDuration(); };

    float getProgress() const override;

    uint32_t getProgressPos() const override;

    void seekTo(float fProgress) override;

    void seekTo(uint32_t seekPos) override;

    float getDuration() const override;

    std::vector<mediakit::Track::Ptr> getTracks(bool ready = true) const override;

    //转ffmpeg的AVCodecID
    int getAVCodecId(mediakit::CodecId id);

private:
    // 派生类回调函数  [AUTO-TRANSLATED:61e20903]
    // Derived class callback function
    bool onCheckSDP(const std::string& sdp) override;

    void onRecvRTP(mediakit::RtpPacket::Ptr rtp, const mediakit::SdpTrack::Ptr& track) override;

    void onPlayResult(const toolkit::SockException& ex) override;

    bool addTrack(const mediakit::Track::Ptr& track) override { return true; }

    void addTrackCompleted() override;

private:
    mediakit::RtspDemuxer::Ptr _demuxer;
};

///////////////////////////////////////////////////
// RtspPlayerImpl

float RtspPlayerImpl::getProgress() const {
    if (getDuration() > 0) {
        return getProgressMilliSecond() / (getDuration() * 1000);
    }
    return PlayerBase::getProgress();
}

uint32_t RtspPlayerImpl::getProgressPos() const {
    if (getDuration() > 0) {
        return getProgressMilliSecond();
    }
    return PlayerBase::getProgressPos();
}

void RtspPlayerImpl::seekTo(float fProgress) {
    fProgress = MAX(float(0), MIN(fProgress, float(1.0)));
    seekToMilliSecond((uint32_t)(fProgress * getDuration() * 1000));
}

void RtspPlayerImpl::seekTo(uint32_t seekPos) {
    seekToMilliSecond(seekPos);
}

float RtspPlayerImpl::getDuration() const {
    return _demuxer ? _demuxer->getDuration() : 0;
}

void RtspPlayerImpl::onPlayResult(const toolkit::SockException& ex) {
    if (!(*this)[mediakit::Client::kWaitTrackReady].as<bool>() || ex) {
        Super::onPlayResult(ex);
        return;
    }
}

void RtspPlayerImpl::addTrackCompleted() {
    if ((*this)[mediakit::Client::kWaitTrackReady].as<bool>()) {
        Super::onPlayResult(toolkit::SockException(toolkit::Err_success, "play success"));
    }
}

std::vector<mediakit::Track::Ptr> RtspPlayerImpl::getTracks(bool ready /*= true*/) const {
    return _demuxer ? _demuxer->getTracks(ready) : Super::getTracks(ready);
}

int RtspPlayerImpl::getAVCodecId(mediakit::CodecId id) {
    //暂时就这几种，后续补充
    switch (id) {
    case mediakit::CodecInvalid: return 0; // AV_CODEC_ID_NONE
    case mediakit::CodecH264: return 27; // AV_CODEC_ID_H264
    case mediakit::CodecH265: return 173; // AV_CODEC_ID_HEVC
    case mediakit::CodecAAC: return 86018; // AV_CODEC_ID_AAC
    case mediakit::CodecG711A: return 65543; // AV_CODEC_ID_PCM_ALAW
    case mediakit::CodecG711U: return 65542; // AV_CODEC_ID_PCM_MULAW
    case mediakit::CodecOpus: return 86076; // AV_CODEC_ID_OPUS
    default: break;
    }
    return 0;
}

bool RtspPlayerImpl::onCheckSDP(const std::string& sdp) {

    _demuxer = std::make_shared<mediakit::RtspDemuxer>();
    _demuxer->setTrackListener(this, (*this)[mediakit::Client::kWaitTrackReady].as<bool>());
    _demuxer->loadSdp(sdp);
    return true;
}

void RtspPlayerImpl::onRecvRTP(mediakit::RtpPacket::Ptr rtp, const mediakit::SdpTrack::Ptr& track) {
    // rtp解复用时可以判断是否为关键帧起始位置  [AUTO-TRANSLATED:fb7d9b6e]
    // When demultiplexing RTP, it can be determined whether it is the starting position of the key frame
    auto key_pos = _demuxer->inputRtp(rtp);
}

#else
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\zlmplayer.lib)")
#endif // USE_ORIGIN_ZLM

#include "log.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}
#include "zlm_player.h"

namespace mp {

RtspPlayer::RtspPlayer()
    : MediaPlayer()
{
}

RtspPlayer::~RtspPlayer()
{
}

void RtspPlayer::Play(const std::string& url, const PlayOptions& options)
{
    MediaPlayer::Play(url, options);
}

void RtspPlayer::Stop()
{
    MediaPlayer::Stop();
}

void RtspPlayer::Pause()
{
    if (m_pZlmPlayer)
        m_pZlmPlayer->Pause();
}

void RtspPlayer::Resume()
{
    if (m_pZlmPlayer)
        m_pZlmPlayer->Resume();
}

void RtspPlayer::Speed(double speed)
{
    MediaPlayer::Speed(speed);
    if (m_pZlmPlayer)
        m_pZlmPlayer->Speed(speed);
}

void RtspPlayer::Seek(int64_t seconds)
{
    if (m_pZlmPlayer)
        m_pZlmPlayer->Seek(seconds);
}

int64_t RtspPlayer::GetDuration()
{
    return 0;
}

static AVCodecID getAVCodecId(int id) {
    //暂时就这几种，后续补充
    switch (id) {
    case -1: return AV_CODEC_ID_NONE;
    case 0: return AV_CODEC_ID_H264;
    case 1: return AV_CODEC_ID_HEVC;
    case 2: return AV_CODEC_ID_AAC;
    case 3: return AV_CODEC_ID_PCM_ALAW;
    case 4: return AV_CODEC_ID_PCM_MULAW;
    case 5: return AV_CODEC_ID_OPUS;
    default: break;
    }
    return AV_CODEC_ID_NONE;
}

static AVSampleFormat getSampleFormat(int sampleBit) {
    switch (sampleBit) {
    case 8: return AV_SAMPLE_FMT_U8;
    case 16: return AV_SAMPLE_FMT_S16;
    case 32: return AV_SAMPLE_FMT_S32;
    default: break;
    }
    return AV_SAMPLE_FMT_NONE;
}

bool RtspPlayer::StreamOpen()
{
#ifndef USE_ORIGIN_ZLM
    m_pZlmPlayer = std::make_shared<zlmplayer::ZlmPlayer>();
    m_pZlmPlayer->SetOnPlayStatus([this](zlmplayer::PlayStatus status) {
        if (status == zlmplayer::PlayStatus::Success) {
            auto videoStream = m_pZlmPlayer->GetVideoStream();
            if (videoStream.codecId < 0) {
                return;
            }
            m_videoTimebae = (double)1 / videoStream.clockRate;
            AVCodecID vCodeId = getAVCodecId(videoStream.codecId);
            CreateVideoDecoder(vCodeId, nullptr);
            auto audioStream = m_pZlmPlayer->GetAudioStream();
            if (audioStream.codecId >= 0) {
                m_audioTimebae = (double)1 / audioStream.clockRate;
                AVCodecID aCodeId = getAVCodecId(audioStream.codecId);
                AVCodecParameters* audioCodecParameters = avcodec_parameters_alloc();
                audioCodecParameters->codec_id = aCodeId;
                audioCodecParameters->codec_type = AVMEDIA_TYPE_AUDIO;
                audioCodecParameters->sample_rate = audioStream.sampleRate;
                audioCodecParameters->format = getSampleFormat(audioStream.sampleBit);
                audioCodecParameters->channels = audioStream.channels;
                audioCodecParameters->channel_layout = av_get_default_channel_layout(audioStream.channels);
                av_channel_layout_default(&audioCodecParameters->ch_layout, audioStream.channels);
                CreateAudioDecoder(aCodeId, audioCodecParameters);
                avcodec_parameters_free(&audioCodecParameters);
            }
        }
        });
    m_pZlmPlayer->SetOnPacket([this](const zlmplayer::Packet& pkt) {
        AVPacket* packet = av_packet_alloc();
        packet->data = pkt.data;
        packet->size = pkt.size;
        if (pkt.isKey) {
            packet->flags |= AV_PKT_FLAG_KEY;
        }
        if (pkt.mediaType == AVMEDIA_TYPE_AUDIO) {
            if (m_firstAudioPts <= 0) {
                m_firstAudioPts = pkt.pts;
            }
            packet->pts = pkt.pts - m_firstAudioPts;
            packet->dts = packet->pts;
            static int packetIndex = 0;
            packetIndex++;
            //LOG_INFO << "audio packet Index:" << packetIndex << ", pts:" << packet->pts;
            if (m_pAudioDecoder->IsOpen() && m_speed == 1.0)
                m_audioPacketQueue.Push(av_packet_clone(packet), true);
        }
        else {
            if (m_firstVideoPts <= 0) {
                m_firstVideoPts = pkt.pts;
            }
            packet->pts = pkt.pts - m_firstVideoPts;
            packet->dts = packet->pts;
            //packet->pts -= 420831312;
            //packet->dts -= 420831312;
            static int packetIndex = 0;
            packetIndex++;
            //LOG_INFO << "video packet Index:" << packetIndex << ", pts:" << packet->pts * m_videoTimebae << ", size:" << packet->size;
            int framePlayInterval = (packet->pts - m_lastVideoPts) * m_videoTimebae * 1000 / m_speed; //帧播放间隔
            //if (pkt.isKey) {
                // 60帧的帧间隔为16.67ms，最大支持60帧
                if (framePlayInterval > 0 && framePlayInterval < 30) {
                    m_isDiscardPacket = true;
                }
                else {
                    m_isDiscardPacket = false;
                }
                //LOG_INFO << "===== start video packet Index:" << packetIndex << ", pts:" << packet->pts << ", framePlayInterval:" << framePlayInterval << ", m_isDiscardPacket:" << m_isDiscardPacket;
            //}

            if (m_speed > 4.0 && !pkt.isKey)
                return;
            //LOG_INFO << "video packet Index:" << packetIndex << ", pts:" << packet->pts << ", ts:" << packet->pts * m_videoTimebae << ", size:" << packet->size;
            //if (m_isDiscardPacket && !pkt.isKey)
            //    return;

            m_lastVideoPts = packet->pts;

            static int packetIndex1 = 0;
            packetIndex1++;
            //LOG_INFO << "===== start video packet Index:" << packetIndex1 << ", pts:" << packet->pts << ", ts:" << packet->pts * m_videoTimebae << ", framePlayInterval:" << framePlayInterval;
            m_videoPacketQueue.Push(av_packet_clone(packet), true);//推送到队列需要拷贝内存，否则所有帧都使用同一块内存解码画面异常
            //if (m_pVideoDecoder)
            //    m_pVideoDecoder->SendPacket(av_packet_clone(packet));
            //LOG_INFO << "===== end video packet Index:" << m_videoPacketIndex;
        }
        av_packet_free(&packet);
        });
    zlmplayer::PlayOptions options;
    options.isTcp = true;
    auto ret = m_pZlmPlayer->Play(m_url, options);
    if (!ret)
        return false;
#else
    auto poller = toolkit::EventPollerPool::Instance().getPoller();
    m_pZlmPlayer = std::make_shared<RtspPlayerImpl>(poller);
    m_pZlmPlayer->setOnPlayResult([this](const toolkit::SockException& ex) {
        LOG_INFO << "OnPlayResult:" << ex.what();
        if (ex) {
            return;
        }
        m_pZlmPlayer->seekTo((uint32_t)0);
        auto videoTrack = std::dynamic_pointer_cast<mediakit::VideoTrack>(m_pZlmPlayer->getTrack(mediakit::TrackVideo));
        if (videoTrack) {
            videoTrack->addDelegate([this](const mediakit::Frame::Ptr& frame) {
                if (frame) {
                    AVPacket* packet = av_packet_alloc();
                    packet->pts = frame->pts();
                    packet->dts = frame->dts();
                    packet->data = (uint8_t*)frame->data();
                    packet->size = frame->size();
                    m_videoPacketIndex++;
                    LOG_INFO << "===== start video packet Index:" << m_videoPacketIndex;
                    m_videoPacketQueue.Push(av_packet_clone(packet), true);//推送到队列需要拷贝内存，否则所有帧都使用同一块内存解码画面异常
                    //if (m_pVideoDecoder)
                    //    m_pVideoDecoder->SendPacket(av_packet_clone(packet));
                    av_packet_free(&packet);
                    LOG_INFO << "===== end video packet Index:" << m_videoPacketIndex;

                }
                return true;
                });

            AVCodecID avCodeId = getAVCodecId(videoTrack->getCodecId());
            m_videoTimebae = 0.001;
            CreateVideoDecoder(avCodeId, nullptr);
        }

        auto audioTrack = std::dynamic_pointer_cast<mediakit::AudioTrack>(m_pZlmPlayer->getTrack(mediakit::TrackAudio));
        if (audioTrack) {
            audioTrack->addDelegate([this](const mediakit::Frame::Ptr& frame) {
                if (frame) {
                    AVPacket* packet = av_packet_alloc();
                    packet->pts = frame->pts();
                    packet->dts = frame->dts();
                    packet->data = (uint8_t*)frame->data();
                    packet->size = frame->size();
                    m_audioPacketIndex++;
                    LOG_INFO << "audio packet Index:" << m_audioPacketIndex;
                    m_audioPacketQueue.Push(av_packet_clone(packet), false);
                    //if (m_pAudioDecoder)
                    //    m_pAudioDecoder->SendPacket(packet);
                }
                return true;
                });

            m_audioTimebae = 0.001;
            AVCodecID avCodeId = getAVCodecId(audioTrack->getCodecId());
            CreateAudioDecoder(avCodeId, nullptr);
        }

        });

    m_pZlmPlayer->setOnShutdown([this](const toolkit::SockException& ex) {
        LOG_ERROR << "OnShutdown:" << ex.what();
        });

    // RTP transport over TCP
    (*m_pZlmPlayer)[mediakit::Client::kRtpType] = mediakit::Rtsp::RTP_TCP;
    m_pZlmPlayer->play(m_url);
#endif // USE_ORIGIN_ZLM
    return true;
}

void RtspPlayer::StreamClose()
{
#ifdef USE_ORIGIN_ZLM
    if (m_pZlmPlayer) {
        m_pZlmPlayer->getTrack(mediakit::TrackVideo)->clear();
        m_pZlmPlayer->getTrack(mediakit::TrackAudio)->clear();
    }
#endif // USE_ORIGIN_ZLM
    MediaPlayer::StreamClose();
    m_firstAudioPts = 0;
    m_firstVideoPts = 0;
    if (m_pZlmPlayer)
        m_pZlmPlayer->Stop();
}

} // namespace mp