#include "fisheye_widget.h"
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include "fisheye_child_widget.h"
#include "media/fisheye_correction.h"

const std::vector<cv::Scalar> kColorList = {
    cv::Scalar(255,255,255), cv::Scalar(255,0,0), cv::Scalar(0,255,0),
    cv::Scalar(0,0,255), cv::Scalar(255,0,0), cv::Scalar(255,0,0),
    cv::Scalar(255,0,0), cv::Scalar(255,0,0), cv::Scalar(255,0,0)
};

FishEyeWidget::FishEyeWidget(QWidget* parent)
    : QWidget(parent)
    , m_pFishEyeCorrection(new FishEyeCorrection())
{
    setStyleSheet("background-color: #000000;");
    setAttribute(Qt::WA_StyledBackground, true);//启用样式表背景
    m_pGridLayout = new QGridLayout(this);
    m_pGridLayout->setSpacing(1);

    connect(this, &FishEyeWidget::sig_Update, this, &FishEyeWidget::on_Update, Qt::QueuedConnection);
}

FishEyeWidget::~FishEyeWidget()
{
    disconnect(this, &FishEyeWidget::sig_Update, this, &FishEyeWidget::on_Update);
}

void FishEyeWidget::SetFishEyeType(FECSetupType eSetupType, FECShowType eShowType)
{
    m_eSetupType = eSetupType;
    m_eShowType = eShowType;

    m_eSetupType = FECSetupType::Top;
    m_eShowType = FECShowType::Panoramic360With1PTZ;

    switch (m_eShowType)
    {
    case FECShowType::Normal:
        break;
    case FECShowType::Panoramic:
        break;
    case FECShowType::PanoramicWith3PTZ:
        break;
    case FECShowType::PanoramicWith8PTZ:
        break;
    case FECShowType::Panoramic180:
        break;
    case FECShowType::Panoramic360:
        break;  
    case FECShowType::Panoramic360With1PTZ: 
    {
        {
            FishEyeChildWidget* pWidget = new FishEyeChildWidget(0, true, this);
            m_listWidgets.push_back(pWidget);
            m_pGridLayout->addWidget(pWidget, 0, 0, 1, 1);
            m_pMainChildWidget = pWidget;
        }
        {
            FishEyeChildWidget* pWidget = new FishEyeChildWidget(1, false, this);
            m_listWidgets.push_back(pWidget);
            m_pGridLayout->addWidget(pWidget, 1, 0, 2, 1);
            pWidget->SetDefaultRotatedRect(0.5f, 90.0f);
        }
        break;
    }

    case FECShowType::Panoramic360With3PTZ:
        break;
    case FECShowType::Panoramic360With6PTZ:
        break;
    case FECShowType::Panoramic360With8PTZ:
        break;
    case FECShowType::FishEyeWith2PTZ:
        break;
    case FECShowType::FishEyeWith3PTZ:
        break;
    case FECShowType::FishEyeWith4PTZ:
        break;
    case FECShowType::FishEyeWith8PTZ:
        break;
    case FECShowType::HalfSphere:
        break;
    case FECShowType::HalfSphereAR:
        break;
    case FECShowType::cylinder:
        break;
    default:
        break;
    }

    return;
    int rows = 2, cols = 2;
    int widgetSize = rows * cols;
    if (widgetSize == 2) {
        //从row行，col列开始，占rowSpan行，占columnSpan列
        {
            FishEyeChildWidget* pWidget = new FishEyeChildWidget(0, true, this);
            m_listWidgets.push_back(pWidget);
            m_pGridLayout->addWidget(pWidget, 0, 0, 1, 1);
            m_pMainChildWidget = pWidget;
        }
        {
            FishEyeChildWidget* pWidget = new FishEyeChildWidget(1, false, this);
            m_listWidgets.push_back(pWidget);
            m_pGridLayout->addWidget(pWidget, 1, 0, 2, 1);
            pWidget->SetDefaultRotatedRect(0.5f, 90.0f);
        }

    }
    else {
        int index = 0;
        for (size_t i = 0; i < rows; i++)
        {
            for (size_t j = 0; j < cols; j++)
            {
                bool isMainChild = (index == 0);
                FishEyeChildWidget* pWidget = new FishEyeChildWidget(index, isMainChild, this);
                m_listWidgets.push_back(pWidget);
                m_pGridLayout->addWidget(pWidget, i, j, 1, 1);
                if (isMainChild) {
                    m_pMainChildWidget = pWidget;
                }
                else {
                    float degree = (2 * index - 1) * 360 / ((widgetSize - 1) * 2);
                    pWidget->SetDefaultRotatedRect(0.5f, degree);
                }
                index++;
            }
        }
    }
}

void FishEyeWidget::UpdateContent(const cv::Mat& content)
{
    if (m_eShowType == FECShowType::Normal) {
        UpdateContentSafe(content.clone());
        return;
    }
    int minSize = std::min(content.cols, content.rows);
    cv::Mat src;
    cv::resize(content, src, cv::Size(minSize, minSize));

    if (m_eShowType == FECShowType::Panoramic180) {
        cv::Mat dst = m_pFishEyeCorrection->GetUnwrapCircular()->GetUnwrapImage(src, minSize / 2);
        dst = dst(cv::Rect(0, minSize * 0.2, dst.cols, dst.rows * 0.8));//移除顶部的数据，因为顶部的数据畸变非常严重
        cv::Mat result = GetLR2TBCroppingImage(dst);
        UpdateContentSafe(result);
        return;
    }
    if (m_eShowType == FECShowType::Panoramic360) {
        cv::Mat dst = m_pFishEyeCorrection->GetUnwrapCircular()->GetUnwrapImage(src, minSize / 2);
        dst = dst(cv::Rect(0, minSize * 0.2, dst.cols, dst.rows * 0.8));//移除顶部的数据，因为顶部的数据畸变非常严重
        UpdateContentSafe(dst);
        return;
    }

    for (size_t i = 0; i < m_listWidgets.size(); i++)
    {
        FishEyeChildWidget* pWidget = m_listWidgets[i];
        pWidget->UpdateOriginSize(minSize, minSize);
        if (!pWidget->IsMainChild()) {
            // 创建旋转矩形
            cv::RotatedRect rotatedRect = pWidget->GetRotatedRect();
            // 提取区域
            cv::Mat extractedRegion = m_pFishEyeCorrection->ExtractRotatedRegionROI(src, rotatedRect);
            pWidget->UpdateContent(extractedRegion);
        }
    }

    for (size_t i = 0; i < m_listWidgets.size(); i++)
    {
        FishEyeChildWidget* pWidget = m_listWidgets[i];
        if (!pWidget->IsMainChild()) {
            // 创建旋转矩形
            cv::RotatedRect rotatedRect = pWidget->GetRotatedRect();
            // 在原图上绘制旋转矩形
            cv::Point2f vertices[4];
            rotatedRect.points(vertices);
            for (int j = 0; j < 4; j++) {
                line(src, vertices[j], vertices[(j + 1) % 4], kColorList[i], 3);
            }
        }
    }

    cv::Mat dst = m_pFishEyeCorrection->GetUnwrapCircular()->GetUnwrapImage(src, minSize / 2);
    //cv::Mat result;
    //cv::flip(dst, result, -1);
    m_pMainChildWidget->UpdateContent(dst);

    //m_contentMutex.lock();
    //m_content = dst(cv::Rect(0, minSize * 0.1, dst.cols, dst.rows - minSize * 0.1));//这里移除顶部10%的数据，因为顶部10%的数据畸变非常严重
    //m_contentMutex.unlock();
    //emit sig_Update();
}

void FishEyeWidget::UpdateContentSafe(const cv::Mat& content)
{
    //这里不使用clone，前面调用时判断是否使用clone
    m_contentMutex.lock();
    m_content = content;
    m_contentMutex.unlock();
    emit sig_Update();
}

void FishEyeWidget::on_Update()
{
    update();
}

void FishEyeWidget::on_ChildMousePress(const QPoint& point)
{
    cv::Point originPoint = m_pFishEyeCorrection->GetUnwrapCircular()->GetOriginPoint(cv::Point(point.x(), point.y()));
    int selectedIndex = -1;
    for (size_t i = 0; i < m_listWidgets.size(); i++)
    {
        FishEyeChildWidget* pWidget = m_listWidgets[i];
        bool isInRegin = m_pFishEyeCorrection->IsPointInRotatedRectTransform(originPoint, pWidget->GetRotatedRect());
        if (isInRegin) {
            selectedIndex = i;
            break;
        }
    }
    m_selectedIndex = selectedIndex;
}

void FishEyeWidget::on_ChildMouseMove(const QPoint& mouseMove)
{
    if (m_selectedIndex < 0)
        return;
    FishEyeChildWidget* pWidget = m_listWidgets[m_selectedIndex];
    pWidget->UpdateRotatedRect(mouseMove.x(), mouseMove.y(), false);
}

void FishEyeWidget::on_ChildMouseRelease(const QPoint& mouseMove)
{
    if (m_selectedIndex < 0)
        return;
    FishEyeChildWidget* pWidget = m_listWidgets[m_selectedIndex];
    pWidget->UpdateRotatedRect(mouseMove.x(), mouseMove.y(), true);
    m_selectedIndex = -1;
}


void FishEyeWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();

        update();
    }

}

void FishEyeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointRelease = event->pos();
        QPoint mouseMove = QPoint(m_pointRelease.x() - m_pointPress.x(), m_pointRelease.y() - m_pointPress.y());
        //qDebug() << "mouseMove" << mouseMove;
        m_lastScrollOffset = m_lastScrollOffset + mouseMove.x();
        update();
    }

}

void FishEyeWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if ((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        m_pointMove = event->pos();
        QPoint mouseMove = QPoint(m_pointMove.x() - m_pointPress.x(), m_pointMove.y() - m_pointPress.y());
        //qDebug() << "mouseMove" << mouseMove;
        m_curScrollOffset = m_lastScrollOffset + mouseMove.x();
        update();
    }

}

void FishEyeWidget::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
    // 获取滚轮滚动的角度差（通常120度为一个"刻度"）
    QPoint angleDelta = event->angleDelta();
    // 获取鼠标位置
    QPoint position = event->pos();
    if (!angleDelta.isNull()) {
        if (angleDelta.y() > 0) {
            // 向上滚动
            qDebug() << "向上滚动，角度:" << angleDelta.y() << "鼠标位置:" << position;
        }
        else if (angleDelta.y() < 0) {
            // 向下滚动
            qDebug() << "向下滚动，角度:" << angleDelta.y() << "鼠标位置:" << position;
        }
        float scale = angleDelta.y() >= 0 ? -0.1f : 0.1f;
        
        // 接受事件，阻止继续传播
        event->accept();
    }
    else {
        // 让基类处理或其他处理
        event->ignore();
    }
}

void FishEyeWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    //geometry()相对于父窗体的rect区域，当窗体是主窗体时，即是屏幕上的位置，客户区。
    //rect()的x()、y()始终从(0, 0)起，宽高客户区宽高。
    //pos()相对于父窗体的位置
    QRect rc = rect();
    if (m_content.empty())
    {
        return;
    }
    m_contentMutex.lock();
    //计算保持宽高比的缩放尺寸
    //scaled方法会申请新的内存空间，可以解锁，如没有申请新的内存空间，需要在drawImage之后才能解锁，避免数据更改
    QImage image = QImage(m_content.data, m_content.cols, m_content.rows, m_content.step, QImage::Format_RGB888)
        .scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_contentMutex.unlock();

    int xPos = (rc.width() - image.width()) / 2;
    int yPos = (rc.height() - image.height()) / 2;
    //painter.drawImage(xPos, yPos, image);

    int drawOffset = m_curScrollOffset % width();
    if (drawOffset != 0) {
        painter.drawImage(QRect(xPos, yPos, drawOffset, image.height()), image, QRect(image.width() - drawOffset, 0, drawOffset, image.height()));
    }
    painter.drawImage(QRect(xPos + drawOffset, yPos, image.width() - drawOffset, image.height()), image, QRect(0, 0, image.width() - drawOffset, image.height()));
}
