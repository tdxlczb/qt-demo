#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include "media/media_define.h"

class VideoRender;
class MediaReader;
class PlayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget* parent = nullptr);
    ~PlayerWidget();

    void StartPlay(const QString& uri);

signals:
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    void HandleVideoFrame(const VideoFrame& frame);
private:
    VideoRender* m_pVideoRender = nullptr;
    MediaReader* m_pMediaReader = nullptr;
};

#endif // PLAYERWIDGET_H
