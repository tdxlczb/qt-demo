#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHash>
#include <QPushButton>

class CommonWidget;
class LiveWidget;
class BaseLiveWidget;
class PlayGLWidget;

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

signals:

private:

    CommonWidget * commonWidget_;
//    LiveWidget * liveWidget_;
    LiveWidget * liveWidget_;
    PlayGLWidget * playGLWidget_;

    QVBoxLayout * vBoxLayout_;
    QHBoxLayout * hBoxLayout_;
    QPushButton * button1_;
    QPushButton * button2_;

};

#endif // MAINWIDGET_H
