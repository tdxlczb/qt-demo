#ifndef LIVEWIDGET_H
#define LIVEWIDGET_H

#include <QWidget>
#include "base_live_widget.h"

class LiveWidget : public BaseLiveWidget
{
    Q_OBJECT
public:
    explicit LiveWidget(QWidget *parent = nullptr);
    ~LiveWidget();

signals:

protected:
    void on_Play();
    void on_Stop();

private:
    uint32_t  widgetId_ = 0;
};

#endif // LIVEWIDGET_H
