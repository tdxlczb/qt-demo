#ifndef GRAPHICS_ITEM_H
#define GRAPHICS_ITEM_H

#include <QWidget>

class GraphicsItem : public QWidget
{
    Q_OBJECT
public:
    explicit GraphicsItem(QWidget *parent = nullptr);

signals:
};

#endif // GRAPHICS_ITEM_H
