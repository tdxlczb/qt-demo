#include "zlm_demuxer.h"
#include "media_log.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

#include "zlm_player.h"
#pragma comment(lib,R"(E:\code\github\czb\ZLMediaKit\release\windows\Debug\Release\zlmplayer.lib)")

namespace mp {

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

ZlmDemuxer::ZlmDemuxer(const std::string& context)
    : MediaDemuxer(context)
{}

ZlmDemuxer::~ZlmDemuxer()
{
    Close();
}

bool ZlmDemuxer::Open(const std::string& url, const PlayOptions& options, DemuxEvent* event)
{
    Close();

    m_url = url;
    m_options = options;
    m_event = event;

    m_pZlmPlayer = std::make_shared<zlmplayer::ZlmPlayer>();
    m_pZlmPlayer->SetOnPlayStatus([this](zlmplayer::PlayStatus status) {
        DemuxStatus ds = DemuxStatus::OpenFailed;
        if (status == zlmplayer::PlayStatus::Success) {
            ds = DemuxStatus::OpenSuccess;
        } else if (status == zlmplayer::PlayStatus::Stop) {
            ds = DemuxStatus::StreamOver;
        }
        m_openStatus.store(status);
        m_cv.notify_all();
        if (m_event) {
            m_event->OnDemuxStatus(ds);
        }
        });
    m_pZlmPlayer->SetOnStream([this](const zlmplayer::StreamInfo& sinfo) {
        if (sinfo.mediaType == AVMEDIA_TYPE_VIDEO) {
            auto videoStream = sinfo;
            if (videoStream.codecId >= 0) {
                AVCodecID vCodeId = getAVCodecId(videoStream.codecId);
                StreamInfo info;
                info.url = m_url;
                info.streamIndex = videoStream.streamIndex;
                info.mediaType = AVMEDIA_TYPE_VIDEO;
                info.codecId = vCodeId;
                info.timebase = { 1,videoStream.clockRate };
                info.fps = videoStream.frameFps;
                info.videoSpec.width = videoStream.width;
                info.videoSpec.height = videoStream.height;
                if (m_event) {
                    AVCodecParameters* codecpar = avcodec_parameters_alloc();
                    codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
                    codecpar->codec_id = vCodeId;
                    codecpar->width = videoStream.width;
                    codecpar->height = videoStream.height;
                    //这里需要使用av_malloc重新拷贝，否则avcodec_parameters_free中释放内存崩溃
                    uint8_t* extra = (uint8_t*)av_malloc(videoStream.extrasize);
                    std::memcpy(extra, videoStream.extradata, videoStream.extrasize);
                    codecpar->extradata = extra;
                    codecpar->extradata_size = videoStream.extrasize;
                    m_event->OnStream(info, codecpar);
                    avcodec_parameters_free(&codecpar);
                }
            }
        } else if (sinfo.mediaType == AVMEDIA_TYPE_AUDIO) {
            auto audioStream = sinfo;
            if (audioStream.codecId >= 0) {
                AVCodecID aCodeId = getAVCodecId(audioStream.codecId);
                StreamInfo info;
                info.url = m_url;
                info.streamIndex = audioStream.streamIndex;
                info.mediaType = AVMEDIA_TYPE_AUDIO;
                info.codecId = aCodeId;
                info.timebase = { 1,audioStream.clockRate };
                if (m_event) {
                    AVCodecParameters* codecpar = avcodec_parameters_alloc();
                    codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
                    codecpar->codec_id = aCodeId;
                    codecpar->sample_rate = audioStream.sampleRate;
                    codecpar->format = getSampleFormat(audioStream.sampleBit);
                    codecpar->channels = audioStream.channels;
                    codecpar->channel_layout = av_get_default_channel_layout(audioStream.channels);
                    uint8_t* extra = (uint8_t*)av_malloc(audioStream.extrasize);
                    std::memcpy(extra, audioStream.extradata, audioStream.extrasize);
                    codecpar->extradata = extra;
                    codecpar->extradata_size = audioStream.extrasize;
                    av_channel_layout_default(&codecpar->ch_layout, audioStream.channels);
                    m_event->OnStream(info, codecpar);
                    avcodec_parameters_free(&codecpar);
                }
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
        packet->pts = pkt.pts;
        packet->dts = pkt.dts;
        this->OnPacket(pkt.mediaType, packet);
        av_packet_free(&packet);

        });
    zlmplayer::PlayOptions zoptions;
    zoptions.isTcp = true;
    auto ret = m_pZlmPlayer->Play(m_url, zoptions);
    if (!ret)
        return false;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait_for(lock, std::chrono::milliseconds(3000), [this]() {
        return m_openStatus.load() != zlmplayer::PlayStatus::None;
        });

    return m_openStatus.load() == zlmplayer::PlayStatus::Success;
}

void ZlmDemuxer::Close()
{
    Stop();
    if (m_pZlmPlayer) {
        m_event = nullptr;
    }
    m_firstAudioPts = 0;
    m_firstVideoPts = 0;
}

void ZlmDemuxer::Start()
{
    //Open的时候已经开始拉流了，这里不做任何处理
}

void ZlmDemuxer::Stop()
{
    if (m_pZlmPlayer) {
        m_pZlmPlayer->Stop();
    }
}

void ZlmDemuxer::Pause()
{
    if (m_pZlmPlayer) {
        m_pZlmPlayer->Pause();
    }
}

void ZlmDemuxer::Resume()
{
    if (m_pZlmPlayer) {
        m_pZlmPlayer->Resume();
    }
}

void ZlmDemuxer::Speed(double speed)
{
    if (m_pZlmPlayer) {
        m_speed = speed;
        m_pZlmPlayer->Speed(speed);
    }
}

void ZlmDemuxer::Seek(int64_t seconds)
{
    if (m_pZlmPlayer) {
        m_pZlmPlayer->Seek(seconds);
    }
}

double ZlmDemuxer::GetSpeed() const
{
    return m_speed;
}

void ZlmDemuxer::OnPacket(int mediaType, AVPacket* packet)
{
    if (mediaType == AVMEDIA_TYPE_AUDIO) {
        packet->stream_index = 1;
        if (m_firstAudioPts <= 0) {
            m_firstAudioPts = packet->pts;
        }
        packet->pts = packet->pts - m_firstAudioPts;
        packet->dts = packet->pts;
        if (m_event) {
            //这里拷贝是因为zlm源数据packet是循环使用一块内存缓冲区，后续使用队列处理packet需要拷贝内存
            auto pkt = av_packet_clone(packet);//当AVPacket的buf为NULL时，会从data里拷贝内存，否则会继续使用buf的内存
            m_event->OnPacket(pkt);
            av_packet_free(&pkt);
        }
    } else if (mediaType == AVMEDIA_TYPE_VIDEO) {
        packet->stream_index = 0;
        if (m_firstVideoPts <= 0) {
            m_firstVideoPts = packet->pts;
        }
        packet->pts = packet->pts - m_firstVideoPts;
        packet->dts = packet->pts;
        if (m_event) {
            //这里拷贝是因为zlm源数据packet是循环使用一块内存缓冲区，后续使用队列处理packet需要拷贝内存
            auto pkt = av_packet_clone(packet);//当AVPacket的buf为NULL时，会从data里拷贝内存，否则会继续使用buf的内存
            m_event->OnPacket(pkt);
            av_packet_free(&pkt);
        }
    }
}

} // namespace mp