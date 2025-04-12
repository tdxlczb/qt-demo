#include "common_widget.h"
#include "live_widget.h"

CommonWidget::CommonWidget(QWidget *parent) : QWidget(parent)
{
//    this->resize(400,500);
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(100, 150, 30));
    this->setPalette(palette);
    this->setAutoFillBackground(true);


//    this->setContentsMargins(1, 1, 1, 1);
//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowTitle("CommonWidget");

    this->setStyleSheet(
                "background-image: url(:/resource/image/logo.png);"  // 资源文件路径
                "background-position: center;"            // 居中
                "background-repeat: no-repeat;"           // 不重复
                "background-color: transparent;"          // 透明背景
                );
}


void CommonWidget::AddLiveWidget(int nLoopNum)
{
    for(int i = nLastGLNumber_; i < nLoopNum; i++) {
        //创建视频显示Widget
        LiveWidget *pWidget = new LiveWidget(this);
        hashLiveWidgets_.insert(i, pWidget);
    }

    pSelectLiveWidgetLast_ = nullptr;
    pSelectLiveWidget_ = hashLiveWidgets_.value(0);
//    on_SetSelectLiveWidget(pSelectLiveWidget_);
}
