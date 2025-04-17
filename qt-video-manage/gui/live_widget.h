#ifndef LIVE_WIDGET_H
#define LIVE_WIDGET_H

#include <QLabel>
#include "base_live_widget.h"


namespace Ui {
class LiveWidget;
}

class LiveWidget : public BaseLiveWidget
{
    Q_OBJECT

public:
    LiveWidget(QWidget *parent, int iWidgetIndex);
    ~LiveWidget();

protected:
    void resizeEvent(QResizeEvent *event);
private:
    Ui::LiveWidget *ui;
    QLabel * m_bgLabel = nullptr;
};

#endif // LIVE_WIDGET_H
