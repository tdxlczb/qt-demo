#include "graphics_widget.h"
#include <QGraphicsEllipseItem>
#include "custom_view.h"
#include "custom_scene.h"

GraphicsWidget::GraphicsWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(1600,900);
    this->setWindowTitle("GraphicsWidget");
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);

    CustomView *pView = new CustomView(this);
    CustomScene *pScene = new CustomScene();
    pScene->setBackgroundBrush(QBrush(QColor(50, 150, 50)));
//    pScene->setSceneRect(0,0,1920, 1080);
    pScene->setSceneRect(0,0,1500, 800);
    pView->setScene(pScene);
    pView->resize(1500,800);
    //pView->setBackgroundBrush(QBrush(QColor(50, 50, 150)));
    pView->show();
    //QGraphicsEllipseItem *ellipse = pScene->addEllipse(QRectF(10, 10, 100, 50), QPen(Qt::black), QBrush(Qt::blue));
}

GraphicsWidget::~GraphicsWidget()
{

}
