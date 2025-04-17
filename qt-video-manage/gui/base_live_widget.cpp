#include "base_live_widget.h"
#include <Windows.h>
#include <QDebug>
#include <QKeyEvent>

BaseLiveWidget::BaseLiveWidget(QWidget *parent, int iWidgetIndex)
    : QWidget(parent)
    , m_iWidgetIndex(iWidgetIndex)
{

}

BaseLiveWidget::~BaseLiveWidget()
{

}

int BaseLiveWidget::GetWidgetIndex() const
{
    return m_iWidgetIndex;
}

void BaseLiveWidget::SetAttachParentHwnd(HWND hwnd)
{
    m_hwndAttachParent = hwnd;
}

bool BaseLiveWidget::SetFullScreen()
{
    if(m_isFullScreen)
        return true;

    if(m_hwndAttachParent)
    {
        if(!SetParent((HWND)this->winId(), nullptr)) {
            qDebug() << "Widget:" << m_iWidgetIndex << " SetParent Error:" << 0;
        }
    }
    //    normalGeometry_ = this->geometry();
    //    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setFocus();
    this->showFullScreen();

    m_isFullScreen = true;
    return true;
}

bool BaseLiveWidget::QuitFullScreen()
{
    if(!m_isFullScreen)
        return true;

    if(m_hwndAttachParent)
    {
        if(!SetParent((HWND)this->winId(), m_hwndAttachParent)) {
            qDebug() << "Widget:" << m_iWidgetIndex << " SetParent Error:" << (qintptr)m_hwndAttachParent;
        }
    }
    //    this->setGeometry(normalGeometry_);
    //    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->showNormal();
    this->show();

    m_isFullScreen = false;
    return true;
}

void BaseLiveWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && m_isFullScreen) {
        // 检测到 ESC 键，退出全屏模式
        QuitFullScreen();
    } else {
        QWidget::keyPressEvent(event);  // 保留默认处理
    }
}
