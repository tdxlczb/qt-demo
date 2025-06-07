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


QString GetMenuStyle()
{
    QString style = R"(
QMenu {
        border: 1px solid #CCCCCC;  /* 边框宽度为1px，颜色为#CCCCCC */
        border-radius: 3px;         /* 边框圆角 */
        background-color: #FAFAFC;  /* 背景颜色 */
        font-size: 10pt;            /* 文本字体大小 */
        padding: 3px 0px 3px 0px;   /* 菜单项距菜单顶部边界和底部边界分别有多少px */
}

QMenu::item { /* 菜单子控件item，为菜单项在default的状态 */
    border: 0px solid transparent;
    background-color: transparent;
    color: #606266; /* 文本颜色 */
    min-height: 35px; /* 菜单项的最小高度 */
    margin: 0 2 0 2;
    padding-left:30;
}

QMenu::icon{
subcontrol-position: left; padding-left:20;
}

QMenu::item:selected { /* 为菜单项在selected的状态 */
    /*background-color: #EDEDEF;*/
    background-color: #F0F2F5;
    color: #606266;
}

QMenu::item:disabled{ /* 为菜单项在disabled的状态 */
    color: #CCCCCC;
    background: none;
}


QMenu::separator { /* 菜单子控件separator，定义菜单项之间的分隔线 */
    height: 1px;
    background: #CCCCCC;
    margin-left: 2px; /* 距离菜单左边界2px */
    margin-right: 2px; /* 距离菜单右边界2px */
}

QMenu::right-arrow { /* 菜单子控件right-arrow，定义子菜单指示器 */
    width: 16px;
    height: 16px;
    image: url(:/icon/resource/icon/Right.png);
    padding-right:10;
}

)";
    return style;
}


CustomBorderMenu::CustomBorderMenu(QWidget* parent)
    : QMenu(parent)
    , m_startX(50)
    , m_triangleWidth(TRIANGLE_WIDTH)
    , m_triangleHeight(TRIANGLE_HEIGHT)
{
    setWindowFlags(Qt::Popup | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(GetMenuStyle());
    
    auto style = styleSheet();
    setStyleSheet(style.append("QMenu {background: #FAFA00; border: 0px; border-radius: 0px; margin: 15px 15px 15px 25px;}"));
    // 设置阴影边框;
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor(255, 0, 0));
    shadowEffect->setBlurRadius(SHADOW_WIDTH);
    this->setGraphicsEffect(shadowEffect);
    //this->setContentsMargins(25,15,15,15);

    // margin 设置为SHADOW_WIDTH + BORDER_RADIUS，左边再加 TRIANGLE_HEIGHT(三角形显示在左右两侧时，可以认为TRIANGLE_HEIGHT为宽，TRIANGLE_WIDTH为高)
    //this->setStyleSheet("QMenu {background: #10f0f8;  margin-left:25px; margin-top:15px; margin-right:15px; margin-bottom:15px; }");
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
    painter.setBrush(QColor(250, 250, 252));

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