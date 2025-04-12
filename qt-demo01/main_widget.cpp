#include "main_widget.h"

#include <windows.h>

#include <QDebug>

#include "common_widget.h"
#include "live_widget.h"
#include "play_glwidget.h"


MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , vBoxLayout_(new QVBoxLayout(this))
    , hBoxLayout_(new QHBoxLayout(this))
{
    this->resize(400,300);
    this->setWindowTitle("MainWidget");
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(250, 150, 30));
//    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);

//    QPixmap pixmap(":/resource/image/logo.png");

//    palette.setBrush(QPalette::Window, QBrush(pixmap));
//    this->setPalette(palette);

//    commonWidget1_ = new CommonWidget(nullptr);
////    commonWidget1_->setLayout(gridLayout1_);

//    commonWidget1_->setGeometry(100,100, 400,200);
//    commonWidget1_->raise();
//    {
//        HWND hwnd = (HWND)commonWidget1_->winId();
//        SetWindowPos(hwnd, HWND_TOP, 0, 0, 600, 400, SWP_NOMOVE );
//    }
////    commonWidget2_->setVisible(true);
//    commonWidget1_->show();


//    this->setStyleSheet(
//                "background-image: url(:/resource/image/logo.png);"  // 资源文件路径
//                "background-position: center;"            // 居中
//                "background-repeat: no-repeat;"           // 不重复
////                "background-color: transparent;"          // 透明背景
//                );


    liveWidget_ = new LiveWidget(nullptr);


    HWND hwndParent = HWND(459960);
    if(!SetParent((HWND)liveWidget_->winId(), hwndParent)) {
        qCritical()<<"SetParent Error";
    }
    liveWidget_->SetAttachParentHwnd(hwndParent);
    liveWidget_->setGeometry(496,364,621,370);
    liveWidget_->show();


    button1_ = new QPushButton("button1",this);
    button2_ = new QPushButton("button2",this);
    vBoxLayout_->addWidget(button1_);
    vBoxLayout_->addWidget(button2_);
    connect(button1_,&QPushButton::clicked,[this, hwndParent](){
        qDebug() << "button1 clicked";

        liveWidget_->SetFullScreen();

    });

    connect(button2_,&QPushButton::clicked,[this, hwndParent](){
        qDebug() << "button2 clicked";

//        liveWidget_->QuitFullScreen();
    });


    playGLWidget_ = new PlayGLWidget();
    playGLWidget_->PlayOneFrame();
    playGLWidget_->show();
}
