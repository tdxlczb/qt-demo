#include "video_render.h"
#include <QPainter>

VideoRender::VideoRender(QWidget* parent) : QWidget(parent)
{
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(0, 0, 0));
    this->setPalette(palette);
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

void VideoRender::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    //geometry()相对于父窗体的rect区域，当窗体是主窗体时，即是屏幕上的位置，客户区。
    //rect()的x()、y()始终从(0, 0)起，宽高客户区宽高。
    //pos()相对于父窗体的位置
    QRect rc = rect();
    if (m_matData.empty())
    {
        //painter.fillRect(rc, QColor(0, 0, 0));
        return;
    }
    m_matDataMutex.lock();
    //计算保持宽高比的缩放尺寸
    QImage scaledImage = QImage(m_matData.data, m_matData.cols, m_matData.rows, m_matData.step, QImage::Format_RGB888)
        .scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_matDataMutex.unlock();

    int xPos = (rc.width() - scaledImage.width()) / 2;
    int yPos = (rc.height() - scaledImage.height()) / 2;
    painter.drawImage(xPos, yPos, scaledImage);
}
