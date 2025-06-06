#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

protected:
    void mouseReleaseEvent(QMouseEvent* event);

private:
    Ui::MainWidget *ui;
};

#endif // MAIN_WIDGET_H
