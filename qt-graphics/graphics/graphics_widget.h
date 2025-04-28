#ifndef GRAPHICSWIDGET_H
#define GRAPHICSWIDGET_H

#include <QWidget>

class GraphicsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphicsWidget(QWidget *parent = nullptr);
    ~GraphicsWidget();
signals:

};

#endif // GRAPHICSWIDGET_H
