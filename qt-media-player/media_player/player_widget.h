#ifndef PLAYER_WIDGET_H
#define PLAYER_WIDGET_H

#include <atomic>
#include <QWidget>
#include <QHash>
#include "media/media_define.h"
#include "media/media_play_event.h"

class VideoRender;
class AudioRender;
class MediaReader;
class FishEyeWidget;
class VideoConverter;
class AudioConverter;
class PlayerWidget : public QWidget, public PlayEvent
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget* parent = nullptr, int winIndex = 0);
    ~PlayerWidget();

    void StartPlay(const QString& url, int decodeType = 0);
    void StopPlay();

protected:
    virtual void onVideoFrame(const VideoFrame& frame) override;
    virtual void onAudioFrame(const AudioFrame& frame) override;
    virtual void onClose(const PlayError& error) override;

signals:
    //通知：该窗口被选中
    void sig_Selected(PlayerWidget* pWidget);

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
    VideoRender* m_pVideoRender = nullptr;
    AudioRender* m_pAudioRender = nullptr;
    MediaReader* m_pMediaReader = nullptr;
    FishEyeWidget* m_pFishEyeWidget = nullptr;

    std::atomic_bool m_isVideoPlaying = {false};
    std::atomic_bool m_isAudioPlaying = {false};

    QHash<int, VideoConverter*> m_hashVideoConverter;
    QHash<int, AudioConverter*> m_hashAudioConverter;
};

#endif // PLAYER_WIDGET_H
