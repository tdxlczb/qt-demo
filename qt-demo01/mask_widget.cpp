#include "mask_widget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QImageReader>
#include <cmath>

MaskWidget::MaskWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);

    // QPalette palette = this->palette();
    // //通常指窗口部件的背景色
    // palette.setColor(QPalette::Window,QColor(50,50,50));
    // this->setPalette(palette);
    // //设置透明度
    // this->setWindowOpacity(0.6f);
    // this->setAttribute(Qt::WA_TranslucentBackground, true);
    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // this->installEventFilter(this);
    // setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool); // 无边框
    setAttribute(Qt::WA_TranslucentBackground);

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
    m_movePoint = event->pos();
    update();
}

void MaskWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // QLinearGradient gradient(0,0,100,100);
    // //初始化渐变对象，左上坐标和宽高设定位置
    // gradient.setColorAt(0.2,Qt::blue);
    // //设定渐变色彩，第一个参数为参照点的位置比例，第二个参数为参照点的颜色
    // gradient.setColorAt(0.5,Qt::red);
    // gradient.setColorAt(0.8,Qt::yellow);
    // painter.setBrush(QBrush(gradient));
    // //QBrush接受渐变对象做参数，并将其作为QPainter的绘制工具
    // painter.drawEllipse(0,0,100,100);

    qDebug() << "start:(" << m_startPoint.x() << ","<<m_startPoint.y() << "), end:(" << m_movePoint.x() << ","<<m_movePoint.y() << ")";
    QRect rc(m_startPoint,m_movePoint);
    if(std::abs(m_startPoint.x() - m_movePoint.x()) > 10 || std::abs(m_startPoint.y() - m_movePoint.y()) > 10)
    {
        painter.setPen(QPen(QColor(150, 50, 50), 5));
        painter.drawRect(rc);

        // painter.fillRect(rc, QColor(150, 50, 50));
        // // 绘制透明层
        // painter.drawRoundedRect(this->rect(), 5, 5);
        // // 设置rec1局部区域透明
        // painter.setCompositionMode(QPainter::CompositionMode_Clear);
        // painter.setBrush(Qt::red);

        // painter.fillRect(rc, Qt::SolidPattern);
        // painter.fillRect(rc, QColor(255, 255, 255, 0));
    }

#if 1
        //把遮罩的某个区域设置为透明
        //画阴影遮罩
        painter.fillRect(this->rect(), QColor(0, 0, 0, 150));

        //最常见的类型是SourceOver（通常称为alpha混合）
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rc, QColor(255, 255, 255, 10));
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
# else

    QString img = ":/res/bg.png";
    QImageReader reader(img);
    QSize size = reader.size();
    resize(size);

    //画背景图片
    painter.drawImage(this->rect(), QImage(img));
    //
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    //叠加透明的矩形区域,以显示图像
    painter.fillRect(rc, QColor(0, 0, 0, 0));
#endif

}


void MaskWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    this->setGeometry(parentWidget()->geometry());
}
