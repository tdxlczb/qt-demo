#ifndef MEDIA_DEMUXER_H
#define MEDIA_DEMUXER_H

#include "media_define.h"
#include "media_context.h"

struct AVCodecParameters;
struct AVPacket;

namespace mp {

enum class DemuxStatus {
    OpenSuccess = 0,
    OpenFailed,
    StreamOver
};

class DemuxEvent
{
public:
    DemuxEvent() {};
    virtual ~DemuxEvent() {};

    virtual void OnDemuxStatus(DemuxStatus status) = 0;
    virtual bool OnStream(const StreamInfo&, AVCodecParameters*) = 0;
    virtual void OnPacket(AVPacket* pkt) = 0;
    virtual void OnPlayError(const PlayError& error) = 0;

};

class MediaDemuxer : public MediaContext
{
public:
    MediaDemuxer(const std::string& context) : MediaContext(context) {};
    virtual ~MediaDemuxer() {};

    virtual bool Open(const std::string& url, const PlayOptions& options, DemuxEvent* event) = 0;
    virtual void Close() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() {};
    virtual void Resume() {};
    virtual void Speed(double speed) {};
    //跳转从流开始时间计算的秒数
    virtual void Seek(int64_t seconds) {};

    virtual double GetSpeed() const { return 1.0; }
    virtual int64_t GetDuration() const { return 0; }

protected:
    DemuxEvent* m_event = nullptr;
};


} // namespace mp

#endif // MEDIA_DEMUXER_H