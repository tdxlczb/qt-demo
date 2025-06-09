#ifndef VIDEORENDER_H
#define VIDEORENDER_H

#include <QWidget>
#include <QMutex>
#include <QImage>
#include <opencv2/core.hpp>

class VideoRender : public QWidget
{
    Q_OBJECT
public:
    explicit VideoRender(QWidget *parent = nullptr);
    ~VideoRender();

    void UpdateContent(const cv::Mat& mat);
signals:

protected:
    void paintEvent(QPaintEvent *event);
private:
    cv::Mat m_matData;
    QMutex m_matDataMutex;
};

#endif // VIDEORENDER_H
