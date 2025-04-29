#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QGraphicsItem>
#include <QGraphicsRectItem>

// QGraphicsItem is not use signals and slots
class CustomItem : public QGraphicsRectItem
{
public:
    explicit CustomItem(QGraphicsItem *parent = nullptr);
    ~CustomItem();

    void MoveTo(const QRectF &rect);
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:

};

#endif // CUSTOMITEM_H
