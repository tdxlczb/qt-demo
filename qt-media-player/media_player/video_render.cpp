#include "video_render.h"
#include <QPainter>

VideoRender::VideoRender(QWidget *parent) : QWidget(parent)
{

}

VideoRender::~VideoRender()
{

}

void VideoRender::UpdateContent(const cv::Mat& mat)
{
    m_matDataMutex.lock();
    m_matData = mat;
    m_matDataMutex.unlock();
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
    m_matDataMutex.lock();
    //计算保持宽高比的缩放尺寸
	QImage scaledImage = QImage(m_matData.data, m_matData.cols, m_matData.rows, m_matData.step, QImage::Format_RGB888)
		.scaled(this->rect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_matDataMutex.unlock();
	painter.drawImage(0, 0, scaledImage);
}
