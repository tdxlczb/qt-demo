#ifndef TEST02_WIDGET_H
#define TEST02_WIDGET_H

#include <QWidget>

namespace Ui {
class Test02Widget;
}

class Test02Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Test02Widget(QWidget *parent = nullptr);
    ~Test02Widget();

private:
    Ui::Test02Widget *ui;
};

#endif // TEST02_WIDGET_H
