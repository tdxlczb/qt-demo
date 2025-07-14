#include "custom_child_widget.h"
#include <QPainter>
#include <QDebug>

CustomChildWidget::CustomChildWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(400, 400);
    this->setWindowTitle("CustomChildWidget");

    //setStyleSheet("background-color: #FF9999;");
    //setAttribute(Qt::WA_StyledBackground,true);//启用样式表背景

    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window,QColor(150,50,50));
    this->setPalette(palette);
}

CustomChildWidget::~CustomChildWidget()
{
    qDebug() << "delete CustomChildWidget";
}

CustomTransparentChildWidget::CustomTransparentChildWidget(QWidget* parent)
    : QWidget(parent)
{
    this->resize(400, 400);
    this->setWindowTitle("CustomTransparentChildWidget");
    //在父窗口界面持续重绘的情况下，显示半透明窗口，必须设置为独立窗口+透明背景
    //半透明背景只能通过重绘paintEvent去实现，其他设置半透明背景的方式都不生效
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);//独立窗口、无边框
    this->setAttribute(Qt::WA_TranslucentBackground, true);//透明背景

}

CustomTransparentChildWidget::~CustomTransparentChildWidget()
{
    qDebug() << "delete CustomTransparentChildWidget";
}

void CustomTransparentChildWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明背景 (最后的 100 表示透明度，取值 0~255)
    QColor bgColor(255, 0, 0, 150);
    painter.fillRect(rect(), bgColor);

    QWidget::paintEvent(event);  // 保证子控件正常绘制
}