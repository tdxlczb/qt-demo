#include "video_render.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
VideoRender::VideoRender(QWidget *parent) : QWidget(parent)
{

}

VideoRender::~VideoRender()
{

}

void VideoRender::UpdateContent(const cv::Mat& mat)
{
    m_matData = mat;
    update();
}

void VideoRender::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    if (m_matData.empty())
    {
        painter.fillRect(geometry(), QColor(50, 50, 50));
        return;
    }
    QImage image = QImage(m_matData.data,m_matData.cols,m_matData.rows,m_matData.step,QImage::Format_RGB888).scaled(QSize(width(),height()));
    painter.drawImage(0,0,image);
}
