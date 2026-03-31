#ifndef ZLM_DEMUXER_H
#define ZLM_DEMUXER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include "media_demuxer.h"

namespace zlmplayer {
class ZlmPlayer;
}

namespace mp {

class ZlmDemuxer : public MediaDemuxer
{
public:
    ZlmDemuxer(const std::string& context);
    ~ZlmDemuxer();

    bool Open(const std::string& url, const PlayOptions& options, DemuxEvent* event) override;
    void Close() override;
    void Start() override;
    void Stop() override;
    void Pause() override;
    void Resume() override;
    void Speed(double speed) override;
    //跳转从流开始时间计算的秒数
    void Seek(int64_t seconds) override;
    double GetSpeed() const override;

private:
    void OnPacket(int mediaType, AVPacket* pkt);
private:
    std::string m_url;
    PlayOptions m_options;
    std::shared_ptr<zlmplayer::ZlmPlayer> m_pZlmPlayer;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::atomic_int m_openStatus{ -1 }; //记录流播放状态，用于播放同步操作
    double m_speed = 1.0;             //记录倍速播放

    int64_t m_firstAudioPts = 0;
    int64_t m_firstVideoPts = 0;
};

} // namespace mp

#endif // ZLM_DEMUXER_H