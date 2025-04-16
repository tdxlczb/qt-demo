#ifndef LIVE_WIDGET_H
#define LIVE_WIDGET_H

#include <QWidget>

namespace Ui {
class LiveWidget;
}

class LiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LiveWidget(QWidget *parent = nullptr);
    ~LiveWidget();

private:
    Ui::LiveWidget *ui;
};

#endif // LIVE_WIDGET_H
