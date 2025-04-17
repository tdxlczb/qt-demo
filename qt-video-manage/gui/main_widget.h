#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
}

class LiveWidget;
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    LiveWidget * m_pLiveWidget = nullptr;
};

#endif // MAIN_WIDGET_H
