#include "base_live_widget.h"
#include <QMovie>
#include <QKeyEvent>
#include <QDebug>

BaseLiveWidget::BaseLiveWidget(QWidget *parent)
    : QWidget(parent)
    , bgLabel_(new QLabel(this))
    , loadingLabel_(new QLabel(this))
    , widgetId_(0)
    , isFullScreen_(false)
    , hwndAttachParent_(NULL)
{
    bgLabel_->setPixmap(QPixmap(":/resource/image/logo.png"));
    bgLabel_->setScaledContents(true);
    //    bgLabel_->hide();


    //    QPalette palette;
    //    palette.setColor(QPalette::Window,QColor(10, 150, 30));
    //    loadingLabel_->setPalette(palette);
    //    loadingLabel_->setAutoFillBackground(true);

//    QMovie *movie = new QMovie(":/resource/image/loading.gif");
//    movie->setScaledSize(QSize(200,200));
//    loadingLabel_->setMovie(movie);
//    //    loadingLabel_->setFixedSize(200,200);
//    //    loadingLabel_->setScaledContents(true);
//    loadingLabel_->setAlignment(Qt::AlignCenter);

//    movie->start();


     this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

BaseLiveWidget::~BaseLiveWidget()
{

}

void BaseLiveWidget::SetAttachParentHwnd(HWND hwnd)
{
    hwndAttachParent_ = hwnd;
}


void BaseLiveWidget::Play()
{

}

void BaseLiveWidget::Stop()
{

}


bool BaseLiveWidget::SetFullScreen()
{
    if(isFullScreen_)
        return true;

    if(hwndAttachParent_)
    {
        if(!SetParent((HWND)this->winId(), nullptr)) {
            qDebug()<<"SetParent Error";
        }
    }
    normalGeometry_ = this->geometry();
//    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setFocus();
    this->showFullScreen();

    isFullScreen_ = true;
    return true;
}

bool BaseLiveWidget::QuitFullScreen()
{
    if(!isFullScreen_)
        return true;

    if(!SetParent((HWND)this->winId(), hwndAttachParent_)) {
        qDebug()<<"SetParent Error：" << (qintptr)hwndAttachParent_;
    }

//    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->showNormal();
    this->setGeometry(normalGeometry_);
    this->show();

    isFullScreen_ = false;
    return true;
}

void BaseLiveWidget::resizeEvent(QResizeEvent *event)
{
    bgLabel_->resize(this->size());
    //手动设置居中
    loadingLabel_->move((width()-200)/2, (height()-200)/2);
}

void BaseLiveWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
}

void BaseLiveWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
}

void BaseLiveWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
}

void BaseLiveWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void BaseLiveWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void BaseLiveWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
}

void BaseLiveWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void BaseLiveWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

void BaseLiveWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && isFullScreen_) {
        // 检测到 ESC 键，退出全屏模式
        QuitFullScreen();
    } else {
        QWidget::keyPressEvent(event);  // 保留默认处理
    }
}
