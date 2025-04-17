#ifndef BASELIVEWIDGET_H
#define BASELIVEWIDGET_H

#include <QWidget>

class BaseLiveWidget : public QWidget
{
    Q_OBJECT
public:
    BaseLiveWidget(QWidget *parent, int iWidgetIndex);
    ~BaseLiveWidget();

    int GetWidgetIndex() const;
    void SetAttachParentHwnd(HWND hwnd);

public:
    virtual bool SetFullScreen();
    virtual bool QuitFullScreen();

signals:
protected:
    virtual void keyPressEvent(QKeyEvent *event);
protected:
    int m_iWidgetIndex = 0;
    HWND m_hwndAttachParent = nullptr;

    bool m_isFullScreen = false;
};

#endif // BASELIVEWIDGET_H
