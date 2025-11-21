#ifndef PLAY_EVENT_H
#define PLAY_EVENT_H

#include "media_define.h"

class PlayEvent
{
public:
    virtual ~PlayEvent() {};
    //// 音频流回调，返回false则不处理音频
    //virtual bool onAudioStream(int steamId, int codec, int samplerate, int channels) = 0;
    //// 视频流回调，返回false则不处理视频，也不会有onVideoFrame回调
    //virtual bool onVideoStream(int steamId, int codec, int width, int height) = 0;
    // 视频帧回调
    virtual void onVideoFrame(const VideoFrame& frame) = 0;
    // 音频帧回调
    virtual void onAudioFrame(const AudioFrame& frame) = 0;
    virtual void onClose(const PlayError& error) = 0;
private:

};

#endif // PLAY_EVENT_H
