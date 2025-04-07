#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QGridLayout>

class CommonWidget;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

signals:

private:
    QGridLayout * gridLayout1_;
    QGridLayout * gridLayout2_;
    CommonWidget * commonWidget1_;
    CommonWidget * commonWidget2_;
};

#endif // MAINWIDGET_H
