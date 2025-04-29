#include "custom_item.h"
#include <QPainter>
#include <QDebug>

CustomItem::CustomItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{

}

CustomItem::~CustomItem()
{
}

void CustomItem::MoveTo(const QRectF &rect)
{
    setPos(mapToScene(rect.topLeft()));
    qDebug() << "set pos:" << rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height();
    update();
}

void CustomItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen = painter->pen();
    pen.setWidth(4);
    pen.setColor(QColor(150,50,50));

    painter->setPen(pen);
    QRectF rc = rect();
    painter->drawRect(rc);
    qDebug() << "paint rect:" << rc.x() << "," << rc.y() << "," << rc.width() << "," << rc.height();
}
