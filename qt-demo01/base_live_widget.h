#ifndef BASELIVEWIDGET_H
#define BASELIVEWIDGET_H

#include <windows.h>

#include <QWidget>
#include <QLabel>


class BaseLiveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseLiveWidget(QWidget *parent = nullptr);
    ~BaseLiveWidget();

    void SetAttachParentHwnd(HWND hwnd);

//signals:

    virtual void Play();
    virtual void Stop();

public:

    virtual bool SetFullScreen();
    virtual bool QuitFullScreen();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
private:
    QLabel * bgLabel_;
    QLabel * loadingLabel_;

    int  widgetId_;
    bool isFullScreen_;
    HWND hwndAttachParent_; //需要附着的窗口句柄，为空表示不附着
    QRect normalGeometry_; //窗口位置
};

#endif // BASELIVEWIDGET_H
