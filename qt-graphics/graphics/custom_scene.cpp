#include "custom_scene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include "custom_item.h"

CustomScene::CustomScene(QObject *parent)
    : QGraphicsScene(parent)
{

}

CustomScene::~CustomScene()
{

}


void CustomScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pPress = event->scenePos();
    QPointF p2 = pPress + QPointF(200,200);
    QRectF rect(pPress,p2);
    //QGraphicsScene::mousePressEvent(event);
    //QGraphicsEllipseItem *ellipse = addEllipse(QRectF(pPress.x(), pPress.y(), 100, 50), QPen(Qt::black), QBrush(Qt::blue));

    //    QGraphicsRectItem * rectItem = new QGraphicsRectItem();
    //    rectItem->setRect(rect);
    //    rectItem->setPen(QPen(Qt::black));
    //    rectItem->setBrush(QBrush(Qt::blue));
    //    addItem(rectItem);

    if(IsHitItem(pPress))
    {
        QGraphicsScene::mousePressEvent(event);
        return;
    }
    CustomItem * rectItem = new CustomItem();
    rectItem->setRect(rect);
    //    rectItem->setPen(QPen(Qt::black));
    //    rectItem->setBrush(QBrush(Qt::blue));
    addItem(rectItem);
}

void CustomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsScene::mouseMoveEvent(event);
    QList<QGraphicsItem*> items = this->items();
    if(items.size() >= 1)
    {
        QPointF pPress = event->scenePos();
        QPointF p2 = pPress + QPointF(200,200);
        QRectF rect(pPress,p2);
        CustomItem * i = dynamic_cast<CustomItem*>(items.at(0));
        i->MoveTo(rect);
    }
    update();
}

void CustomScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsScene::mouseReleaseEvent(event);
}


bool CustomScene::IsHitItem(QPointF pt)
{
    QList<QGraphicsItem*> items = this->items(pt);
    foreach (QGraphicsItem *item, items)
    {
        CustomItem * i = dynamic_cast<CustomItem*>(item);
        if(!i){
            continue;
        }
        if(i->contains(pt))
        {
            return true;
        }
    }
    return false;
}
