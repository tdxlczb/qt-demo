#include "painter_widget.h"
#include <QtMath>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

PainterWidget::PainterWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(1600,900);
    this->setWindowTitle("PainterWidget");
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);
    this->setAutoFillBackground(true);
}

PainterWidget::~PainterWidget()
{
}

void PainterWidget::StartDraw()
{
    m_bStartDraw = true;
}

void PainterWidget::StopDraw()
{
    m_bStartDraw = false;
}

void PainterWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    if(event->button() == Qt::MouseButton::LeftButton) {
        m_startPoint = event->pos();
    }

}

void PainterWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton) {
        m_endPoint = event->pos();
        m_movePoint = m_endPoint;
        update();
    }
}

void PainterWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {//判断只有左键按下移动
        m_movePoint = event->pos();
        update();
    }
}

void PainterWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    if(!m_bStartDraw)
        return;

    DrawVerticalArrow();
}

void PainterWidget::DrawRect()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 163, 255), 10, Qt::DotLine));

    QRect rc(m_startPoint,m_movePoint);
    if(std::abs(m_startPoint.x() - m_movePoint.x()) > 10 || std::abs(m_startPoint.y() - m_movePoint.y()) > 10)
    {
        painter.drawRect(rc);
    }
}

void PainterWidget::DrawLineArrow()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 163, 255), 5, Qt::SolidLine));
    painter.setBrush(QColor(0, 163, 255));

    qreal arrowSize = 100;
    qreal arrowAngle = M_PI / 3;
    QLineF line(m_startPoint, m_movePoint);
//    QLineF line(QPointF(100,100), QPointF(500,400));
    double angle = std::atan2(line.dy(), line.dx());
    double degrees = qRadiansToDegrees(angle);
    QPointF arrowP1 = line.p2() + QPointF(cos(M_PI + angle + arrowAngle / 2) * arrowSize,
                                          sin(M_PI + angle + arrowAngle / 2) * arrowSize);
    QPointF arrowP2 = line.p2() + QPointF(cos(M_PI + angle - arrowAngle / 2) * arrowSize,
                                          sin(M_PI + angle - arrowAngle / 2) * arrowSize);
    QPolygonF arrowHead;
    arrowHead.clear();
    arrowHead << line.p2() << arrowP1 << arrowP2;
    painter.drawLine(line);
    painter.drawPolygon(arrowHead);
}


void PainterWidget::DrawVerticalLine()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 163, 255), 5, Qt::SolidLine));

    qreal lineSize = 100;
    QLineF line(m_startPoint, m_movePoint);
    int dx = line.dx();
    int dy = line.dy();
    double angle = std::atan2(line.dy(), line.dx());
    double degrees = qRadiansToDegrees(angle);
    QPointF p11 = QPointF(cos(angle - M_PI/2) * lineSize/2,  sin(angle - M_PI/2) * lineSize/2);
    QPointF p22 = QPointF(cos(angle + M_PI/2) * lineSize/2,  sin(angle + M_PI/2) * lineSize/2);
    QPointF p1 = line.p2() + QPointF(p11.x(),  p11.y());
    QPointF p2 = line.p2() + QPointF(p22.x(),  p22.y());
    QPolygonF arrowHead;
    arrowHead.clear();
    arrowHead << p1 << p2;
    painter.drawLine(line);
    painter.drawPolygon(arrowHead);
}


void PainterWidget::DrawVerticalArrow()
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 163, 255), 10, Qt::SolidLine));

    qreal lineSize = 100;
    qreal arrowSize = 20;
    qreal arrowAngle = M_PI/3; //60度
    QLineF mouseMoveLine(m_startPoint, m_movePoint);
    double mouseMoveLineAngle = std::atan2(mouseMoveLine.dy(), mouseMoveLine.dx());

    QPointF verticalLineP1 = mouseMoveLine.center() + QPointF(cos(mouseMoveLineAngle - M_PI/2) * lineSize/2,  sin(mouseMoveLineAngle - M_PI/2) * lineSize/2);
    QPointF verticalLineP2 = mouseMoveLine.center() + QPointF(cos(mouseMoveLineAngle + M_PI/2) * lineSize/2,  sin(mouseMoveLineAngle + M_PI/2) * lineSize/2);
    QLineF  verticalLine(verticalLineP1, verticalLineP2);

    double VerticalLineAngle1 = std::atan2(verticalLine.dy(),verticalLine.dx()) + M_PI;
    double VerticalLineAngle2 = std::atan2(verticalLine.dy(),verticalLine.dx());

    QPointF verticalLineP1Arrow1 = verticalLine.p1() + QPointF(cos(M_PI + VerticalLineAngle1 - arrowAngle/2) * arrowSize,  sin(M_PI + VerticalLineAngle1 - arrowAngle/2) * arrowSize);
    QPointF verticalLineP1Arrow2 = verticalLine.p1() + QPointF(cos(M_PI + VerticalLineAngle1 + arrowAngle/2) * arrowSize,  sin(M_PI + VerticalLineAngle1 + arrowAngle/2) * arrowSize);
    QPointF verticalLineP2Arrow1 = verticalLine.p2() + QPointF(cos(M_PI + VerticalLineAngle2 - arrowAngle/2) * arrowSize,  sin(M_PI + VerticalLineAngle2 - arrowAngle/2) * arrowSize);
    QPointF verticalLineP2Arrow2 = verticalLine.p2() + QPointF(cos(M_PI + VerticalLineAngle2 + arrowAngle/2) * arrowSize,  sin(M_PI + VerticalLineAngle2 + arrowAngle/2) * arrowSize);
    QPolygonF arrowHead1;
    arrowHead1.clear();
    arrowHead1 << verticalLineP1Arrow1 << verticalLine.p1()<<verticalLineP1Arrow2;
    QPolygonF arrowHead2;
    arrowHead2.clear();
    arrowHead2 << verticalLineP2Arrow1 << verticalLine.p2()<<verticalLineP2Arrow2;
    painter.drawLine(mouseMoveLine);
    painter.drawLine(verticalLine);
    painter.drawPolygon(arrowHead1);
    painter.drawPolygon(arrowHead2);
}

