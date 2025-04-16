#ifndef BASELIVEWIDGET_H
#define BASELIVEWIDGET_H

#include <windows.h>

#include <QWidget>
#include <QLabel>


class BaseLiveWidget : public QWidget
{
    Q_OBJECT
public:
    BaseLiveWidget(QWidget *parent, int widgetId);
    ~BaseLiveWidget();

    void SetAttachParentHwnd(HWND hwnd);

    //signals:

    virtual void Play();
    virtual void Stop();

public:

    virtual bool SetFullScreen();
    virtual bool QuitFullScreen();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void moveEvent(QMoveEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    QLabel * bgLabel_ = nullptr;
    QLabel * loadingLabel_ = nullptr;
    QLabel * bgLabel2_ = nullptr;

    int  widgetId_ = 0;
    bool isFullScreen_ = false;
    HWND hwndAttachParent_ = nullptr; //需要附着的窗口句柄，为空表示不附着
    QRect normalGeometry_; //窗口位置
};

#endif // BASELIVEWIDGET_H
