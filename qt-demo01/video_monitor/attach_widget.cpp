#include "attach_widget.h"
#include <QPainter>
#include <QDebug>
#include <Windows.h>
#include "window_listener.h"
#include "monitor_widget.h"

AttachWidget::AttachWidget(QWidget* parent) : QWidget(parent)
{
    this->resize(100, 100);
    this->setWindowTitle("AttachChildWidget");

    setWindowFlags(Qt::Window       // 必须是顶层窗口
        | Qt::Tool                  // 不显示在任务栏
        | Qt::FramelessWindowHint); // 无边框

    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(150, 50, 50));
    this->setPalette(palette);
    m_listener = new WindowListener();

    connect(&m_timer, &QTimer::timeout, this, &AttachWidget::UpdateChild);
}

AttachWidget::~AttachWidget()
{
    qDebug() << "delete AttachChildWidget";
    delete m_listener;
}

bool AttachWidget::Attach(HWND parentHwnd)
{
    HWND hwnd = (HWND)winId();
    if (!SetParent(hwnd, parentHwnd)) {
        qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        return false;
    }
    ::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

    m_target = parentHwnd;
    m_timer.start(40);
    //if (m_listener) {
    //    m_listener->start(hwnd);
    //}
    return true;
}

void AttachWidget::Detach()
{
    m_timer.stop();
    if (m_listener) {
        m_listener->stop();
    }
}

void AttachWidget::AddChild(QWidget* child)
{
    m_childList.append(child);
}

void AttachWidget::UpdateChild()
{
    bool isMinimized = IsIconic(m_target);
    bool isVisible = IsWindowVisible(m_target);
    RECT targetRect;
    GetWindowRect(m_target, &targetRect);
    RECT currentRect;
    GetWindowRect((HWND)winId(), &currentRect);
    qInfo() << "isMinimized" << isMinimized << "isVisible" << isVisible << "x" << currentRect.left << "y" << currentRect.top << "w" << currentRect.right - currentRect.left << "h" << currentRect.bottom - currentRect.top;

    QRect rect(targetRect.left, targetRect.top, targetRect.right - targetRect.left, targetRect.bottom - targetRect.top);
    bool isHide = isMinimized || !isVisible;
    bool isMove = m_lastRect != rect;
    for (size_t i = 0; i < m_childList.size(); i++)
    {
        auto child = m_childList.at(i);
        bool childHide = child->isHidden();
        bool childVisible = child->isVisible();
        if (!isHide && childVisible) {
            child->show();
        } else if (isHide && childVisible) {
            child->hide();
        }

        if (isMove) {
            auto oldrc = child->geometry();
            child->move(rect.left() + oldrc.left(), rect.top() + oldrc.top());
        }
    }
    m_lastRect = rect;
}

AttachWidget2::AttachWidget2(QWidget* parent) : QWidget(parent)
{
    //this->resize(800, 800);
    //this->setWindowTitle("AttachWidget2");

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    //this->setWindowFlags(Qt::Window // 必须是顶层窗口
    //    | Qt::Tool                  // 不显示在任务栏
    //    | Qt::FramelessWindowHint); // 无边框

    this->setWindowOpacity(0.99);

    qInfo() << "Create AttachWidget2 Handle:" << (int)winId();
}

AttachWidget2::~AttachWidget2()
{
    qDebug() << "delete AttachWidget2";
}

bool AttachWidget2::Attach(HWND parentHwnd)
{
    HWND hwnd = (HWND)winId();
    if (!SetParent(hwnd, parentHwnd)) {
        qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        return false;
    }
    //::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

    auto pMonitor = new MonitorWidget();
    if (!SetParent((HWND)pMonitor->winId(), hwnd)) {
        qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        return false;
    }
    //pMonitor->move(10, 10);
    //pMonitor->resize(800, 800);
    pMonitor->setGeometry(10, 10, 600, 600);
    pMonitor->show();
    return true;
}

void AttachWidget2::Detach()
{

}

void AttachWidget2::moveEvent(QMoveEvent* event)
{
    qInfo() << "moveEvent";
}

void AttachWidget2::resizeEvent(QResizeEvent* event)
{
    qInfo() << "resizeEvent";
}

void AttachWidget2::closeEvent(QCloseEvent* event)
{
    qInfo() << "closeEvent";
}

void AttachWidget2::showEvent(QShowEvent* event)
{
    qInfo() << "showEvent";
}

void AttachWidget2::hideEvent(QHideEvent* event)
{
    qInfo() << "hideEvent";
}

bool AttachWidget2::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    MSG* msg = static_cast<MSG*>(message);
    //qInfo() << "nativeEvent:" << eventType << ", message:" << msg->message;
    switch (msg->message) {
    case WM_MOVE:
        qInfo() << "移动";
        break;
    case WM_SIZE:
        switch (msg->wParam) {
        case SIZE_MINIMIZED:
            qInfo() << "最小化";
            break;
        case SIZE_MAXIMIZED:
            qInfo() << "最大化";
            break;
        case SIZE_RESTORED:
            qInfo() << "还原/resize";
            break;
        }
        break;
    case WM_SHOWWINDOW:
        if (msg->wParam)
            qInfo() << "显示";
        else
            qInfo() << "隐藏";
        break;
    }
    return QWidget::nativeEvent(eventType, message, result);
}