#ifndef CUSTOMCHILDWIDGET_H
#define CUSTOMCHILDWIDGET_H

#include <QWidget>

class CustomChildWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomChildWidget(QWidget *parent = nullptr);
    ~CustomChildWidget();
signals:

};

class CustomTransparentChildWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTransparentChildWidget(QWidget* parent = nullptr);
    ~CustomTransparentChildWidget();
signals:

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // CUSTOMCHILDWIDGET_H
