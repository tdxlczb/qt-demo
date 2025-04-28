#include "graphics_widget.h"
#include <QGraphicsEllipseItem>
#include "graphics_view.h"
#include "graphics_scene.h"

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


    GraphicsScene *pScene = new GraphicsScene(this);
    GraphicsView *pView = new GraphicsView(pScene, this);

    QGraphicsEllipseItem *ellipse = pScene->addEllipse(QRectF(10, 10, 100, 50), QPen(Qt::black), QBrush(Qt::blue));
}

GraphicsWidget::~GraphicsWidget()
{

}
