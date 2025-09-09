#include "fisheye_widget.h"
#include <QDebug>
#include <QLabel>
#include <QPainter>
#include "media/fisheye_correction.h"

FishEyeChildWidget::FishEyeChildWidget(int index, bool isMainChild, FishEyeWidget* parent)
    : QWidget(parent)
    , m_index(index)
    , m_isMainChild(isMainChild)
{
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(100, 100, 100));
    this->setPalette(palette);
    QLabel* label = new QLabel(this);
    label->setText("TestLabel");
    connect(this, &FishEyeChildWidget::sig_Update, this, &FishEyeChildWidget::on_Update, Qt::QueuedConnection);
    if (m_isMainChild) {
        connect(this, &FishEyeChildWidget::sig_ChildMousePress, parent, &FishEyeWidget::on_ChildMousePress, Qt::QueuedConnection);
        connect(this, &FishEyeChildWidget::sig_ChildMouseMove, parent, &FishEyeWidget::on_ChildMouseMove, Qt::QueuedConnection);
        connect(this, &FishEyeChildWidget::sig_ChildMouseRelease, parent, &FishEyeWidget::on_ChildMouseRelease, Qt::QueuedConnection);
    }
}

FishEyeChildWidget::~FishEyeChildWidget()
{
    disconnect(this, &FishEyeChildWidget::sig_Update, this, &FishEyeChildWidget::on_Update);
}

bool FishEyeChildWidget::IsMainChild()
{
    return m_isMainChild;
}

void FishEyeChildWidget::SetSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

bool FishEyeChildWidget::IsSelected()
{
    return m_isSelected;
}

void FishEyeChildWidget::SetDefaultRotatedRect(float rotateRadiusScale, float rotateDegree)
{
    m_defaultRotateRadiusScale = rotateRadiusScale;
    m_defaultRotateAngle = DegreesToRadians(rotateDegree);
}

cv::RotatedRect FishEyeChildWidget::GetRotatedRect()
{
    if (m_originWidth == 0 || m_originHeight == 0)
        return cv::RotatedRect();

    if (m_lastRotateRadius == 0) {
        m_lastRotateRadius = m_defaultRotateRadiusScale * GetOriginRadius();
        m_lastRotateAngle = m_defaultRotateAngle;
        UpdateRotatedRect(m_lastRotateRadius, m_lastRotateAngle);
    }
    return m_rotatedRect;
}

void FishEyeChildWidget::UpdateOriginSize(int originWidth, int originHeight)
{
    m_originWidth = originWidth;
    m_originHeight = originHeight;
}

void FishEyeChildWidget::UpdateRotatedRect(int rotateRadius, float rotateAngle)
{
    if (m_originWidth == 0 || m_originHeight == 0)
        return;

    int srcWidth = m_originWidth;
    int srcHeight = m_originHeight;
    CheckRotateRadius(rotateRadius);
    cv::Size2f rectSize(srcWidth / 4.0f * m_rotateRectScale, srcHeight / 4.0f * m_rotateRectScale);//矩形大小
    cv::Point2f imageCenter(srcWidth / 2.0f, srcHeight / 2.0f);//图片中心点
    cv::Point2f rectCenter(imageCenter.x + rotateRadius * cos(rotateAngle), imageCenter.y + rotateRadius * sin(rotateAngle));//矩形的中心点
    //qDebug() << "rectCenter:x=" << rectCenter.x << ",y=" << rectCenter.y;
    // 计算旋转角度（使上下边与中心连线垂直）
    cv::Point2f direction = rectCenter - imageCenter;
    float angle = atan2(direction.y, direction.x) * 180.0f / CV_PI + 90.0f;
    cv::RotatedRect rotatedRect = cv::RotatedRect(rectCenter, rectSize, angle);
    m_rotatedRect = rotatedRect;
}

void FishEyeChildWidget::UpdateRotatedRect(int moveX, int moveY, bool isRelease)
{
    int rotateRadius = m_lastRotateRadius + moveY;
    CheckRotateRadius(rotateRadius);
    float rotateAngle = m_lastRotateAngle + GetSectorAngleRadians(moveX, GetOriginRadius());
    UpdateRotatedRect(rotateRadius, rotateAngle);
    if (isRelease) {
        m_lastRotateRadius = rotateRadius;
        m_lastRotateAngle = rotateAngle;
    }
}

void FishEyeChildWidget::UpdateContent(const cv::Mat& content)
{
    m_contentMutex.lock();
    m_content = content.clone();
    m_contentMutex.unlock();
    emit sig_Update();
}

void FishEyeChildWidget::on_Update()
{
    update();
}


void FishEyeChildWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointPress = event->pos();
        m_pointMove = event->pos();
        if (m_isMainChild) {
            int x = round(m_pointPress.x() / m_scaleX);
            int y = round(m_pointPress.y() / m_scaleY) + m_content.rows * 0.2;
            emit sig_ChildMousePress(QPoint(x, y));
        }
        //update();
    }

}

void FishEyeChildWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pointRelease = event->pos();
        QPoint mouseMove = QPoint(m_pointRelease.x() - m_pointPress.x(), m_pointRelease.y() - m_pointPress.y());
        //qDebug() << "mouseMove" << mouseMove;
        if (m_isMainChild) {
            int x = round(mouseMove.x() / m_scaleX);
            int y = round(mouseMove.y() / m_scaleY);
            emit sig_ChildMouseRelease(QPoint(x, y));
        }
        else {
            m_lastRotateRadius = m_lastRotateRadius + mouseMove.y();
            CheckRotateRadius(m_lastRotateRadius);
            //m_lastRotateAngle = m_lastRotateAngle + GetSectorAngleRadians(mouseMove.x(), m_lastRotateRadius);
            m_lastRotateAngle = m_lastRotateAngle + GetSectorAngleRadians(mouseMove.x(), GetOriginRadius()) * 2;//提高移动效率
            UpdateRotatedRect(m_lastRotateRadius, m_lastRotateAngle);
        }
        //update();
    }

}

void FishEyeChildWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void FishEyeChildWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if ((event->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        m_pointMove = event->pos();
        QPoint mouseMove = QPoint(m_pointMove.x() - m_pointPress.x(), m_pointMove.y() - m_pointPress.y());
        //qDebug() << "mouseMove" << mouseMove;
        if (m_isMainChild) {
            int x = round(mouseMove.x() / m_scaleX);
            int y = round(mouseMove.y() / m_scaleY);
            emit sig_ChildMouseMove(QPoint(x, y));
        }
        else {
            int rotateRadius = m_lastRotateRadius + mouseMove.y();
            CheckRotateRadius(rotateRadius);
            //float rotateAngle = m_lastRotateAngle + GetSectorAngleRadians(mouseMove.x(), rotateRadius);
            float rotateAngle = m_lastRotateAngle + GetSectorAngleRadians(mouseMove.x(), GetOriginRadius()) * 2;//提高移动效率
            UpdateRotatedRect(rotateRadius, rotateAngle);
        }
        //update();
    }

}

void FishEyeChildWidget::wheelEvent(QWheelEvent* event)
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
        m_rotateRectScale += scale;
        if (m_rotateRectScale <= 0.2f)
            m_rotateRectScale = 0.2f;
        if (m_rotateRectScale >= 1.0f)
            m_rotateRectScale = 1.0f;
        UpdateRotatedRect(m_lastRotateRadius, m_lastRotateAngle);
        // 接受事件，阻止继续传播
        event->accept();
    }
    else {
        // 让基类处理或其他处理
        event->ignore();
    }
}

void FishEyeChildWidget::paintEvent(QPaintEvent* event)
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
    cv::Mat dst;
    if (m_isMainChild) {
        dst = m_content(cv::Rect(0, m_content.rows * 0.2, m_content.cols, m_content.rows * 0.8));
    }
    else {
        dst = m_content;
    }
    QImage image = QImage(dst.data, dst.cols, dst.rows, dst.step, QImage::Format_RGB888)
        .scaled(rc.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_scaleX = image.size().width() / (float)dst.cols;
    m_scaleY = image.size().height() / (float)dst.rows;
    m_contentMutex.unlock();

    painter.drawImage(0, 0, image);
}

void FishEyeChildWidget::CheckRotateRadius(int& radius)
{
    int originRadius = GetOriginRadius();
    cv::Size2f rectSize = m_rotatedRect.size;
    //移动的半径保持在0.125~0.875之间
    //if (radius > originRadius * kMaxRadiusScale) {
    //    radius = originRadius * kMaxRadiusScale;
    //}
    if (radius < originRadius * kMinRadiusScale) {
        radius = originRadius * kMinRadiusScale;
    }
    if (radius > originRadius - rectSize.height / 2) {
        radius = originRadius - rectSize.height / 2;
    }
    //if (radius < rectSize.height / 2) {
    //    radius = rectSize.height / 2;
    //}
}

int FishEyeChildWidget::GetOriginRadius()
{
    return m_originHeight / 2;
}

const std::vector<cv::Scalar> kColorList = {
    cv::Scalar(255,255,255), cv::Scalar(255,0,0), cv::Scalar(0,255,0),
    cv::Scalar(0,0,255), cv::Scalar(255,0,0), cv::Scalar(255,0,0),
    cv::Scalar(255,0,0), cv::Scalar(255,0,0), cv::Scalar(255,0,0)
};

FishEyeWidget::FishEyeWidget(QWidget* parent)
    : QWidget(parent)
    , m_pFishEyeCorrection(new FishEyeCorrection())
{
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(200, 200, 200));
    this->setPalette(palette);

    m_pGridLayout = new QGridLayout(this);
    m_pGridLayout->setSpacing(1);

    connect(this, &FishEyeWidget::sig_Update, this, &FishEyeWidget::on_Update, Qt::QueuedConnection);
}

FishEyeWidget::~FishEyeWidget()
{
    disconnect(this, &FishEyeWidget::sig_Update, this, &FishEyeWidget::on_Update);
}

void FishEyeWidget::SetFishEyeType(int iSetupType, int iShowType)
{
    m_iSetupType = iSetupType;
    m_iShowType = iShowType;

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
    int minSize = std::min(content.cols, content.rows);
    cv::Mat src;
    cv::resize(content, src, cv::Size(minSize, minSize));

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
    m_pMainChildWidget->UpdateContent(dst);

    //m_contentMutex.lock();
    //m_content = dst(cv::Rect(0, minSize * 0.1, dst.cols, dst.rows - minSize * 0.1));//这里移除顶部10%的数据，因为顶部10%的数据畸变非常严重
    //m_contentMutex.unlock();
    //emit sig_Update();
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

    painter.drawImage(0, 0, image);
}
