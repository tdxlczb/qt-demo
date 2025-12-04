#ifndef RTSP_PLAYER_HPP
#define RTSP_PLAYER_HPP

#include <string>
#include "media_player.h"

//#define USE_ORIGIN_ZLM

#ifdef USE_ORIGIN_ZLM
class RtspPlayerImpl;
#else
namespace zlmplayer {
class ZlmPlayer;
}
#endif // USE_ORIGIN_ZLM

namespace mp {

class RtspPlayer : public MediaPlayer
{
public:
    RtspPlayer();
    ~RtspPlayer();

    void Play(const std::string& url, const PlayOptions& options) override;
    void Stop() override;
    void Pause() override;
    void Resume() override;
    void Speed(double speed) override;
    void Seek(int64_t seconds) override;
    int64_t GetDuration() override;
private:
    bool StreamOpen() override;
    void StreamClose() override;
private:
#ifdef USE_ORIGIN_ZLM
    std::shared_ptr<RtspPlayerImpl> m_pZlmPlayer;
#else
    std::shared_ptr<zlmplayer::ZlmPlayer> m_pZlmPlayer;
    int64_t m_firstAudioPts = 0;
    int64_t m_firstVideoPts = 0;
    int64_t m_lastVideoPts = 0;
    bool m_isDiscardPacket = false; //是否丢帧
#endif // USE_ORIGIN_ZLM
};

} // namespace mp

#endif // RTSP_PLAYER_HPP
