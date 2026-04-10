#include "window_listener.h"
#include <QDebug>

//static HWINEVENTHOOK g_hook = NULL;
//static HWND g_target = NULL;
//static HWND g_overlay = NULL;
//
//void CALLBACK WinEventProc(HWINEVENTHOOK hook,
//    DWORD event,
//    HWND hwnd,
//    LONG idObject,
//    LONG idChild,
//    DWORD dwEventThread,
//    DWORD dwmsEventTime)
//{
//    // 只关心目标窗口
//    if (hwnd != g_target) return;
//
//    qInfo() << "event:" << event;
//
//    // ---- 1. 同步位置 ----
//    if (event == EVENT_OBJECT_LOCATIONCHANGE ||
//        event == EVENT_SYSTEM_FOREGROUND)
//    {
//        RECT rect;
//        if (GetWindowRect(g_target, &rect)) {
//            SetWindowPos(g_overlay,
//                HWND_TOP,
//                0,
//                0,
//                0,
//                0,
//                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
//            ShowWindow(g_overlay, SW_SHOWNORMAL);
//        }
//    }
//
//    // ---- 2. 窗口销毁 ----
//    if (event == EVENT_OBJECT_DESTROY) {
//        ShowWindow(g_overlay, SW_HIDE);
//    }
//}
//
//void installHook(HWND target, HWND overlay)
//{
//    g_target = target;
//    g_overlay = overlay;
//
//    g_hook = SetWinEventHook(
//        EVENT_MIN,   // 最小事件
//        EVENT_MAX, // 最大事件（范围监听）
//        NULL,
//        WinEventProc,
//        0, 0,
//        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
//    );
//}
//
//void uninstallHook()
//{
//    if (g_hook) {
//        UnhookWinEvent(g_hook);
//        g_hook = NULL;
//    }
//}

// 用来绑定窗口与监听实例的唯一标识
constexpr const wchar_t* LISTENER_PROP = L"WindowListenerPtr";

WindowListener::WindowListener(QObject* parent) : QObject(parent)
{
}

WindowListener::~WindowListener()
{
    stop();
}

void WindowListener::start(HWND targetHwnd)
{
    stop();
    m_target = targetHwnd;

    // 把 this 指针存到系统窗口属性里
    SetProp(m_target, LISTENER_PROP, this);

    // 监听：位置变化 + 显示 + 隐藏 + 销毁
    if (!m_eventHook) {
        m_eventHook = SetWinEventHook(
            EVENT_MIN,
            EVENT_MAX,
            NULL,
            WinEventProc,
            0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );
    }

    if (!m_msgHook) {
        //m_msgHook = SetWindowsHookEx(
        //    WH_CALLWNDPROC,
        //    WinMsgProc,
        //    GetModuleHandle(NULL),
        //    GetCurrentThreadId()
        //);
    }
}

void WindowListener::stop()
{
    if (m_eventHook) {
        UnhookWinEvent(m_eventHook);
        m_eventHook = nullptr;
    }
    if (m_msgHook) {
        UnhookWindowsHookEx(m_msgHook);
        m_msgHook = nullptr;
    }
    if (m_target) {
        RemoveProp(m_target, LISTENER_PROP);
        m_target = nullptr;
    }
}

// ==================== 核心回调（无单例） ====================
void CALLBACK WindowListener::WinEventProc(HWINEVENTHOOK hook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime)
{
    //if (idObject != OBJID_WINDOW || idChild != 0)
    //    return;

    if (hwnd == NULL)
        return;

    // 从窗口属性中 取出 this 指针（关键！）
    WindowListener* listener = (WindowListener*)GetProp(hwnd, LISTENER_PROP);
    if (!listener) return;

    if (hwnd != listener->targetHwnd())
        return;

    bool isMinimized = IsIconic(hwnd);
    bool isVisible = IsWindowVisible(hwnd);  // 关键：判断真的可见！

    qInfo() << "HWND:" << (qintptr)hwnd << ", event:" << event << ", isMinimized:" << isMinimized << ", isVisible" << isVisible;

    // 直接调用类成员！
    switch (event) {
    case EVENT_OBJECT_SHOW:
        qInfo() << "show";
        emit listener->sigShow();
        break;
    case EVENT_OBJECT_HIDE:
        qInfo() << "hide";
        emit listener->sigHide();
        break;
    case EVENT_OBJECT_DESTROY:
        qInfo() << "destory";
        emit listener->sigClose();
        break;
    case EVENT_OBJECT_LOCATIONCHANGE:
        qInfo() << "move";
        emit listener->sigMoved();
        emit listener->sigResized();
        break;
    case EVENT_SYSTEM_MINIMIZESTART:
        qInfo() << "min start";
        emit listener->sigHide();
        break;
    case EVENT_SYSTEM_MINIMIZEEND:
        qInfo() << "min end";
        emit listener->sigShow();
        break;
    }
}

LRESULT CALLBACK WindowListener::WinMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        MSG* msg = (MSG*)lParam;
        HWND hwnd = msg->hwnd;

        if (hwnd == NULL)
            return CallNextHookEx(NULL, nCode, wParam, lParam);

        // 从窗口属性取出当前实例
        WindowListener* listener = (WindowListener*)GetProp(hwnd, LISTENER_PROP);
        if (!listener) return 0;

        bool isMinimized = IsIconic(hwnd);
        bool isVisible = IsWindowVisible(hwnd);  // 关键：判断真的可见！
        qInfo() << "HWND:" << (qintptr)hwnd << ", message:" << msg->message << ", isMinimized:" << isMinimized << ", isVisible" << isVisible;

        if (listener && listener->m_target == hwnd)
        {
            switch (msg->message)
            {
                // 显示/隐藏（真·触发）
            case WM_SHOWWINDOW:
                break;

                // 移动
            case WM_MOVE:
                break;

                // 大小变化
            case WM_SIZE:
                break;

                // 关闭/销毁
            case WM_CLOSE:
            case WM_DESTROY:
                break;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
