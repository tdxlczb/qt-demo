#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>

namespace Ui {
class MainWidget;
}

class PainterWidget;
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    PainterWidget * m_pPainterWidget = nullptr;
};

#endif // MAIN_WIDGET_H
