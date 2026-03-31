#ifndef FFMPEG_DEMUXER_H
#define FFMPEG_DEMUXER_H

#include <thread>
#include <atomic>
#include "media_demuxer.h"

struct AVFormatContext;

namespace mp {

class FFmpegDemuxer : public MediaDemuxer
{
public:
    FFmpegDemuxer(const std::string& context);
    ~FFmpegDemuxer();

    bool Open(const std::string& url, const PlayOptions& options, DemuxEvent* event) override;
    void Close() override;
    void Start() override;
    void Stop() override;
    int64_t GetDuration() const override;

private:
    void DemuxThread();

private:
    std::string m_url;
    PlayOptions m_options;
    AVFormatContext* m_formatContext = nullptr;
    std::thread m_thDemuxer;
    std::atomic_bool m_threadRun{ false }; //atomic在gcc编译器中不可使用=进行初始化
    std::atomic_bool m_isStreamOver{ false };

    std::atomic_bool m_seekReq{ false };
    int64_t m_seekPos = 0;
    std::atomic_bool m_pauseReq{ false };
    std::atomic_bool m_isPaused{ false };

};

} // namespace mp

#endif // FFMPEG_DEMUXER_H