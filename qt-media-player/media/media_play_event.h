#ifndef PLAY_EVENT_H
#define PLAY_EVENT_H

#include "media_define.h"

class PlayEvent
{
public:
    virtual ~PlayEvent() {};
    // 视频帧(渲染)回调
    virtual void onVideoFrame(const VideoFrame& frame) = 0;
    virtual void onClose(const PlayError& error) = 0;
private:

};

#endif // PLAY_EVENT_H
