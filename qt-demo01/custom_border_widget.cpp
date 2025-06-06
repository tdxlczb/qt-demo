#include "custom_border_widget.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

#define SHADOW_WIDTH 15 // 窗口阴影宽度;
#define TRIANGLE_WIDTH 15 // 小三角的宽度;
#define TRIANGLE_HEIGHT 10 // 小三角的高度;
#define BORDER_RADIUS 5 // 窗口边角的弧度;

CustomBorderWidget::CustomBorderWidget(QWidget* parent)
    : QWidget(parent)
    , m_startX(50)
    , m_triangleWidth(TRIANGLE_WIDTH)
    , m_triangleHeight(TRIANGLE_HEIGHT)
{
    setWindowFlags(Qt::Popup| Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 设置阴影边框;
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor(255, 0, 0));
    shadowEffect->setBlurRadius(SHADOW_WIDTH);
    this->setGraphicsEffect(shadowEffect);
    //setFixedSize(150, 200);
}

void CustomBorderWidget::setCenterWidget(QWidget* widget)
{
    QHBoxLayout* hMainLayout = new QHBoxLayout(this);
    hMainLayout->addWidget(widget);
    hMainLayout->setSpacing(0);
    hMainLayout->setContentsMargins(SHADOW_WIDTH, SHADOW_WIDTH + TRIANGLE_HEIGHT, SHADOW_WIDTH, SHADOW_WIDTH);
    this->setFixedSize(widget->width() + SHADOW_WIDTH * 2, widget->height() + SHADOW_WIDTH * 2);
}

// 设置小三角显示的起始位置;
void CustomBorderWidget::setStartPos(int startX)
{
    m_startX = startX;
}

void CustomBorderWidget::setTriangleInfo(int width, int height)
{
    m_triangleWidth = width;
    m_triangleHeight = height;
}

void CustomBorderWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    // 小三角区域;
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(m_startX, m_triangleHeight + SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startX + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
    QPainterPath drawPath;
    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH, \
        width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight), \
        BORDER_RADIUS, BORDER_RADIUS);
    // Rect + Triangle;
    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);

    //QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);

    //// 绘制半透明背景
    //painter.fillRect(rect(), QColor(0, 0, 0, 150));  // 黑色半透明

    //// 绘制内容（可选）
    //painter.setPen(Qt::white);
    //painter.drawText(rect(), Qt::AlignCenter, "透明Popup窗口");
}



CustomBorderMenu::CustomBorderMenu(QWidget* parent)
    : QMenu(parent)
    , m_startX(50)
    , m_triangleWidth(TRIANGLE_WIDTH)
    , m_triangleHeight(TRIANGLE_HEIGHT)
{
    setWindowFlags(Qt::Popup | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 设置阴影边框;
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor(255, 0, 0));
    shadowEffect->setBlurRadius(SHADOW_WIDTH);
    this->setGraphicsEffect(shadowEffect);
    this->setStyleSheet("QMenu {background: #10f0f8; padding-top:20px; margin-left:20px; margin-top:30px; margin-right:20px; margin-bottom:20px; }");
    setFixedSize(200, 200);
}

// 设置小三角显示的起始位置;
void CustomBorderMenu::setStartPos(int startX)
{
    m_startX = startX;
}

void CustomBorderMenu::setTriangleInfo(int width, int height)
{
    m_triangleWidth = width;
    m_triangleHeight = height;
}

void CustomBorderMenu::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    // 小三角区域;
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(m_startX, m_triangleHeight + SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startX + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
    QPainterPath drawPath;
    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH, \
        width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight), \
        BORDER_RADIUS, BORDER_RADIUS);
    // Rect + Triangle;
    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);

    QMenu::paintEvent(event);

}