#ifndef PLAYER_WIDGET_H
#define PLAYER_WIDGET_H

#include <atomic>
#include <QWidget>
#include "media/media_define.h"
#include "media/media_play_event.h"

class VideoRender;
class AudioRender;
class MediaReader;
class FishEyeWidget;
class PlayerWidget : public QWidget, public PlayEvent
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget* parent = nullptr);
    ~PlayerWidget();

    void StartPlay(const QString& url);
    void StopPlay();

protected:
    virtual void onVideoFrame(const VideoFrame& frame) override;
    virtual void onClose(const PlayError& error) override;

signals:
private:
    void resizeEvent(QResizeEvent* event) override;

private:
    VideoRender* m_pVideoRender = nullptr;
    AudioRender* m_pAudioRender = nullptr;
    MediaReader* m_pMediaReader = nullptr;
    FishEyeWidget* m_pFishEyeWidget = nullptr;

    std::atomic_bool m_isVideoPlaying = {false};
    std::atomic_bool m_isAudioPlaying = {false};
};

#endif // PLAYER_WIDGET_H
