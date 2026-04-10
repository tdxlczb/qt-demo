#ifndef WINDOW_LISTENER_H
#define WINDOW_LISTENER_H
#pragma once
#include <QObject>
#include <windows.h>

class WindowListener : public QObject
{
    Q_OBJECT
public:
    explicit WindowListener(QObject* parent = nullptr);
    ~WindowListener();

    // 开始监听第三方窗口
    void start(HWND targetHwnd);
    void stop();
    HWND targetHwnd() const { return m_target; }

signals:
    void sigMoved();       // 窗口移动
    void sigResized();     // 窗口大小变化
    void sigShow();        // 显示
    void sigHide();        // 隐藏/最小化
    void sigClose();       // 关闭

private:
    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hook, DWORD event, HWND hwnd,
        LONG idObj, LONG idChild, DWORD idThread, DWORD dwTime
    );
    static LRESULT CALLBACK WinMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    HWND m_target = nullptr;
    HWINEVENTHOOK m_eventHook = nullptr;
    HHOOK  m_msgHook = nullptr;
    static WindowListener* m_instance;
};

#endif // WINDOW_LISTENER_H
