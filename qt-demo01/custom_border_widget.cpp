#include "custom_border_widget.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

#define SHADOW_WIDTH 10 // 窗口阴影宽度;
#define TRIANGLE_WIDTH 20 // 小三角的宽度;
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
    // margin 设置为SHADOW_WIDTH + BORDER_RADIUS，左边再加 TRIANGLE_HEIGHT(三角形显示在左右两侧时，可以认为TRIANGLE_HEIGHT为宽，TRIANGLE_WIDTH为高)
    this->setStyleSheet("QMenu {background: #10f0f8;  margin-left:25px; margin-top:15px; margin-right:15px; margin-bottom:15px; }");
    //this->setStyleSheet("QMenu {background: #10f0f8; padding-top:20px; margin-left:20px; margin-top:30px; margin-right:20px; margin-bottom:20px; }");
    setFixedSize(200, 200);
}

void CustomBorderMenu::setCursorPos(const QPoint& pt)
{
    m_cursorPos = pt;
}

void CustomBorderMenu::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));

    QPoint menuPos = pos();
    int posY =  m_cursorPos.y() - menuPos.y() + SHADOW_WIDTH;

    // 小三角区域;
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(SHADOW_WIDTH, posY);
    trianglePolygon << QPoint(m_triangleHeight + SHADOW_WIDTH, posY + m_triangleWidth / 2);
    trianglePolygon << QPoint(m_triangleHeight + SHADOW_WIDTH, posY - m_triangleWidth / 2);
    QPainterPath drawPath;
    drawPath.addRoundedRect(QRect(m_triangleHeight + SHADOW_WIDTH, SHADOW_WIDTH, \
        width() - SHADOW_WIDTH * 2 - m_triangleHeight, height() - SHADOW_WIDTH * 2), \
        BORDER_RADIUS, BORDER_RADIUS);

    //QPolygon trianglePolygon;
    //trianglePolygon << QPoint(m_startX, m_triangleHeight + SHADOW_WIDTH);
    //trianglePolygon << QPoint(m_startX + m_triangleWidth / 2, SHADOW_WIDTH);
    //trianglePolygon << QPoint(m_startX + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
    //QPainterPath drawPath;
    //drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH, \
    //    width() - SHADOW_WIDTH * 2, height() - SHADOW_WIDTH * 2 - m_triangleHeight), \
    //    BORDER_RADIUS, BORDER_RADIUS);
    // Rect + Triangle;
    drawPath.addPolygon(trianglePolygon);
    painter.drawPath(drawPath);

    //先绘制边框，然后绘制中间内容，使用margin设置边框大小
    QMenu::paintEvent(event);

}