#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <atomic>
#include <QWidget>
#include <QTextEdit>
#include "media/media_define.h"

class VideoRender;
class AudioRender;
class MediaReader;
class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget* parent = nullptr);
    ~PlayerWidget();

    void StartPlay(const QString& url);
    void StopPlay();
signals:
private:
    void on_pbOpenFileButton_clicked();
    void on_pbPlayButton_clicked();
    void on_pbStopButton_clicked();

    void resizeEvent(QResizeEvent* event) override;
    void HandleVideoFrame(const VideoFrame& frame);
    void HandleAudioFrame(const AudioFrame& frame);
private:
    VideoRender* m_pVideoRender = nullptr;
    AudioRender* m_pAudioRender = nullptr;
    MediaReader* m_pMediaReader = nullptr;

    QString m_playUrl;
    std::atomic_bool m_isVideoPlaying = false;
    std::atomic_bool m_isAudioPlaying = false;

    QTextEdit* m_pTextEditUrl = nullptr;
};

#endif // PLAYERWIDGET_H
