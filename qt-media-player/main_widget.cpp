#include "main_widget.h"
#include "ui_main_widget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "media_player/player_widget.h"
#include "media_player/media_player.h"
#include "media_player/video_manager.h"


MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("MainWidget");

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    setLayout(vBoxLayout);

    QPushButton * button01 = new QPushButton("button01", this);
    QPushButton * button02 = new QPushButton("button02", this);

    vBoxLayout->addWidget(button01);
    vBoxLayout->addWidget(button02);

    MediaPlayer* pMediaPlayer = new MediaPlayer(nullptr);
    pMediaPlayer->show();

    VideoManager* pVideoManager = new VideoManager(nullptr);
    pVideoManager->show();

    //PlayerWidget* pPlayerWidget = new PlayerWidget(nullptr, 0);
    //pPlayerWidget->show();

    connect(button01,&QPushButton::clicked,[this](){
        qDebug() << "button01 clicked";
    });

    connect(button02,&QPushButton::clicked,[this](){
        qDebug() << "button02 clicked";
    });

}

MainWidget::~MainWidget()
{
    delete ui;
}
