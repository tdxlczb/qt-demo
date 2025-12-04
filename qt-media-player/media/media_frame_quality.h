#ifndef MEDIA_FRAME_QUALITY_H
#define MEDIA_FRAME_QUALITY_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include "media_define.h"

namespace mp {

class VideoConverter;
//帧质量检测
class FrameQuality
{
public:
    FrameQuality();
    ~FrameQuality();

    bool IsGrayFrame(const VideoFrame& frame);
private:
    std::unique_ptr<VideoConverter> m_rgbConverter;
};

} // namespace mp

#endif // MEDIA_FRAME_QUALITY_H
