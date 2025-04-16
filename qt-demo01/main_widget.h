#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHash>
#include <QVector>
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

    CommonWidget * commonWidget_ = nullptr;
//    LiveWidget * liveWidget_ = nullptr;
    LiveWidget * liveWidget_ = nullptr;
    PlayGLWidget * playGLWidget_ = nullptr;

    QVBoxLayout * vBoxLayout_ = nullptr;
    QHBoxLayout * hBoxLayout_ = nullptr;
    QGridLayout * gridLayout_ = nullptr;
    QPushButton * button1_ = nullptr;
    QPushButton * button2_ = nullptr;
    QVector<QPushButton*> vecButtons_;

};

#endif // MAINWIDGET_H
