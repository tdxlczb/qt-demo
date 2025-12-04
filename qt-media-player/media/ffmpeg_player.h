#ifndef FFMPEG_PLAYER_H
#define FFMPEG_PLAYER_H

#include "media_player.h"

struct AVFormatContext;

namespace mp {

//使用ffmpeg拉流播放

class FFmpegPlayer : public MediaPlayer
{
public:
    FFmpegPlayer();
    ~FFmpegPlayer();

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
    void StreamDemux() override;
private:
    AVFormatContext* m_formatContext = nullptr;
};

} // namespace mp

#endif // FFMPEG_PLAYER_H