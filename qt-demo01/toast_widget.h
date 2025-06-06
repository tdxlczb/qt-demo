#ifndef TOAST_WIDGET_H
#define TOAST_WIDGET_H

#include <QWidget>

class ToastWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToastWidget(QWidget *parent = nullptr);

signals:
};

#endif // TOAST_WIDGET_H
