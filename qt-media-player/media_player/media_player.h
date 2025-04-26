#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <QWidget>

namespace Ui {
class MediaPlayer;
}

class MediaPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

private:
    Ui::MediaPlayer *ui;
};

#endif // MEDIA_PLAYER_H
