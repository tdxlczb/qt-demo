#include "main_window.h"
#include "main_widget.h"

#include "websocket_server.h"

#include "ui_main_window.h"

#include <QMovie>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(400,300);
//    //设置主窗口背景颜色
//    QPalette palette;
//    palette.setColor(QPalette::Window,QColor(255, 150, 30));
//    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
//    this->setPalette(palette);

    mainWidget_ = new MainWidget(nullptr);
//    mainWidget_->setGeometry(0, 0, 300, 100);
    mainWidget_->show();

//    movie_ = new QMovie(this);
//    movie_->setFileName(":/resource/image/loading2.gif");
//    movie_->setCacheMode(QMovie::CacheAll);

//    loadingLabel_->setGeometry(100,100,100,100);
//    movie_->setScaledSize(loadingLabel_->size());

//    loadingLabel_->setMovie(movie_);
//    loadingLabel_->setScaledContents(true);
//    loadingLabel_->setAlignment(Qt::AlignCenter);
//    movie_->start();
//    loadingLabel_->show();

    server_ = new WebSocketServer();
    server_->Init();
}

MainWindow::~MainWindow()
{
    delete ui;
}
