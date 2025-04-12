#include "live_widget.h"
#include <QDebug>

LiveWidget::LiveWidget(QWidget *parent) : BaseLiveWidget(parent)
{
    this->resize(400,300);
//    QPalette palette;
//    palette.setColor(QPalette::Window,QColor(20, 255, 255));
//    this->setPalette(palette);
//    this->setAutoFillBackground(true);

    this->setWindowTitle("LiveWidget");

}

LiveWidget::~LiveWidget()
{
    qDebug() << "delete live widget";
}
