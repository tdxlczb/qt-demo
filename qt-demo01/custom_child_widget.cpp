#include "custom_child_widget.h"

CustomChildWidget::CustomChildWidget(QWidget *parent) : QWidget(parent)
{
    //setStyleSheet("background-color: #FF9999;");
    //setAttribute(Qt::WA_StyledBackground,true);//启用样式表背景

    this->resize(200,200);
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window,QColor(150,50,50));
    this->setPalette(palette);

}
