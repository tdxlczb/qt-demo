#include "common_widget.h"

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
}
