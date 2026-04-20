#ifndef PLAYER_WIDGET_H
#define PLAYER_WIDGET_H

#include <atomic>
#include <QWidget>
#include <QHash>
#include "media/media_define.h"
#include "media/media_play_event.h"

namespace mp {
class MediaPlayer;
class VideoConverter;
class AudioConverter;
class FrameQuality;
}
class VideoRender;
class AudioRender;
class FishEyeWidget;

class PlayerWidget : public QWidget, public mp::PlayEvent
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget* parent = nullptr, int winIndex = 0);
    ~PlayerWidget();

    void StartPlay(const QString& url, int decodeType = 0);
    void StopPlay();
    void PlayPause();
    void ChangeSpeed(double speed);
    void SeekPercent(int value);
    void SeekTime(int value);

protected:
    virtual void onVideoFrame(const mp::VideoFrame& frame) override;
    virtual void onAudioFrame(const mp::AudioFrame& frame) override;
    virtual void onClose(const mp::PlayError& error) override;


signals:
    void sig_PlayTime(int64_t seconds, int64_t totalSeconds);
    //通知：该窗口被选中
    void sig_Selected(PlayerWidget* pWidget);

private:
    void OnUpdateTime(double pts);
private:
    void mousePressEvent(QMouseEvent* event) override;
    //void mouseReleaseEvent(QMouseEvent* event) override;
    //void mouseDoubleClickEvent(QMouseEvent* event) override;
    //void mouseMoveEvent(QMouseEvent* event) override;
    //void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    //void enterEvent(QEvent* event) override;
    //void leaveEvent(QEvent* event) override;
    //void showEvent(QShowEvent* event) override;
    //void hideEvent(QHideEvent* event) override;
    //void dragMoveEvent(QDragMoveEvent* event) override;
    //void dragEnterEvent(QDragEnterEvent* event) override;
    //void dropEvent(QDropEvent* event) override;
    //void paintEvent(QPaintEvent* event) override;
    //void moveEvent(QMoveEvent* event) override;

private:
    int m_winIndex = 0;
    std::string m_playUrl;
    VideoRender* m_pVideoRender = nullptr;
    AudioRender* m_pAudioRender = nullptr; 
    FishEyeWidget* m_pFishEyeWidget = nullptr;
    mp::FrameQuality* m_pFrameQuality = nullptr;

    std::atomic_bool m_isVideoPlaying = { false };
    std::atomic_bool m_isAudioPlaying = { false };

    bool m_isPaused = false;
    double m_playStartPts = 0.0;
    double m_playCurrentPts = 0.0;
    double m_totalSeekTime = 0.0;
    QHash<int, mp::VideoConverter*> m_hashVideoConverter;
    QHash<int, mp::AudioConverter*> m_hashAudioConverter;
};

#endif // PLAYER_WIDGET_H
