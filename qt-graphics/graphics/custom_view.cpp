#include "custom_view.h"

CustomView::CustomView(QWidget *parent)
    : QGraphicsView(parent)
{
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(150, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);
    //    setStyleSheet("background-color: #00e0e0;");
}

CustomView::~CustomView()
{

}

void CustomView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
}

void CustomView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void CustomView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}
