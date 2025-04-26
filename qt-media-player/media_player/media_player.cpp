#include "media_player.h"
#include "ui_media_player.h"

MediaPlayer::MediaPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayer)
{
    ui->setupUi(this);
}

MediaPlayer::~MediaPlayer()
{
    delete ui;
}
