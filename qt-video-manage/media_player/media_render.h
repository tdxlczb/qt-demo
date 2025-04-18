#ifndef MEDIA_RENDER_H
#define MEDIA_RENDER_H

#include <QWidget>
#include <QMutex>

class MediaRender : public QWidget
{
    Q_OBJECT
public:
    explicit MediaRender(QWidget *parent = nullptr);

signals:


};

#endif // MEDIA_RENDER_H
