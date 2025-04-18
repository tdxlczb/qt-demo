#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <QWidget>

class MediaPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MediaPlayer(QWidget *parent = nullptr);

signals:
};

#endif // MEDIA_PLAYER_H
