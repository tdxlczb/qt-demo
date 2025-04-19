#include "mask_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QImageReader>
#include <cmath>

MaskWidget::MaskWidget(QWidget *parent)
    : QWidget{parent}
{
    // QPalette palette = this->palette();
    // //通常指窗口部件的背景色
    // palette.setColor(QPalette::Window,QColor(50,50,50));
    // this->setPalette(palette);
    // //设置透明度，这里是设置窗口整体的透明度
    // this->setWindowOpacity(0.6f);

    // 无边框，并且设置窗口属性透明
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

MaskWidget::~MaskWidget()
{

}

void MaskWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    if(event->button() == Qt::MouseButton::LeftButton) {
        m_startPoint = event->pos();
    }

}

void MaskWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    if(event->button() == Qt::MouseButton::LeftButton) {
        m_endPoint = event->pos();
        m_movePoint = m_endPoint;
        update();
    }
}

void MaskWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void MaskWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {//判断只有左键按下移动
        m_movePoint = event->pos();
        update();
    }
}

void MaskWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing, true);
    //设置绘制混合模式
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    //画阴影遮罩
    painter.fillRect(this->rect(), QColor(178, 178, 178, 150));
    qDebug() << "pos:(" << rect().x() << "," << rect().y() << "," << rect().width() << "," << rect().height() << ")";
    qDebug() << "start:(" << m_startPoint.x() << ","<<m_startPoint.y() << "), end:(" << m_movePoint.x() << ","<<m_movePoint.y() << ")";
    QRect rc(m_startPoint,m_movePoint);
    if(std::abs(m_startPoint.x() - m_movePoint.x()) > 10 || std::abs(m_startPoint.y() - m_movePoint.y()) > 10)
    {
        painter.setPen(QPen(QColor(0, 163, 255), 10, Qt::DotLine));
        painter.drawRect(rc);

        //把遮罩的某个区域设置为透明
        painter.fillRect(rc, QColor(255, 255, 255, 0));
    }
    // QWidget::paintEvent(event);
}


void MaskWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QRect parentRc = parentWidget()->geometry();
    QPoint globalPos = parentWidget()->mapToGlobal(QPoint(0,0));
    QRect rc = QRect(globalPos.x(), globalPos.y(), parentRc.width(), parentRc.height());
    qDebug() << "parent pos:(" << rc.x() << "," << rc.y() << "," << rc.width() << "," << rc.height() << ")";
    this->setGeometry(rc);
}
