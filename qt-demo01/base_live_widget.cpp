#include "base_live_widget.h"
#include <QMovie>
#include <QKeyEvent>
#include <QDebug>

BaseLiveWidget::BaseLiveWidget(QWidget *parent,int widgetId)
    : QWidget(parent)
    , bgLabel_(new QLabel(this))
    , bgLabel2_(new QLabel(this))
    , loadingLabel_(new QLabel(this))
    , widgetId_(widgetId)
    , isFullScreen_(false)
    , hwndAttachParent_(NULL)
{

    QPalette palette;
    palette.setColor(QPalette::Window,QColor(50, 50, 50));
//    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    bgLabel_->setPixmap(QPixmap(":/resource/image/logo.png"));
    bgLabel_->setScaledContents(true);
    //    bgLabel_->hide();


    bgLabel2_->hide();


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
    bgLabel_->hide();
    QPixmap pix("E:\\code\\media\\1.jpg");
    bgLabel2_->setPixmap(pix);
    bgLabel2_->show();
    QResizeEvent event(size(), size());
    resizeEvent(&event);

}

void BaseLiveWidget::Stop()
{
    bgLabel2_->hide();
    bgLabel_->show();
//    update();
    QResizeEvent event(size(), size());
    resizeEvent(&event);
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


void BaseLiveWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mouse clicked";
    QWidget::mousePressEvent(event);
}

void BaseLiveWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void BaseLiveWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void BaseLiveWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void BaseLiveWidget::wheelEvent(QWheelEvent *event)
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

void BaseLiveWidget::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

void BaseLiveWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
}

void BaseLiveWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
}

void BaseLiveWidget::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void BaseLiveWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
}

void BaseLiveWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void BaseLiveWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
}

void BaseLiveWidget::resizeEvent(QResizeEvent *event)
{
    bgLabel_->resize(this->size());
    bgLabel2_->resize(this->size());
    //手动设置居中
    loadingLabel_->move((width()-200)/2, (height()-200)/2);
}

void BaseLiveWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}
