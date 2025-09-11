#include "graphics.h"
#include "cv_utils.h"

// 构造函数：支持普通扇形和圆环扇形
CircularSector::CircularSector(cv::Point center, float startAngle, float endAngle, int outerRadius, int innerRadius)
    : m_center(center)
    , m_startAngle(startAngle)
    , m_endAngle(endAngle)
    , m_outerRadius(outerRadius)
    , m_innerRadius(innerRadius)
{
    // 确保半径有效
    m_outerRadius = std::max(0, m_outerRadius);
    m_innerRadius = std::max(0, std::min(m_innerRadius, m_outerRadius));
}

CircularSector::~CircularSector()
{

}

// 绘制扇形/圆环扇形边框
void CircularSector::DrawBorder(cv::Mat& image, const cv::Scalar& color, int thickness) const {
    if (image.empty()) return;

    // 绘制外圆弧
    cv::ellipse(image, m_center, cv::Size(m_outerRadius, m_outerRadius), 0,
        m_startAngle, m_endAngle, color, thickness);

    // 如果有内半径，绘制内圆弧
    if (m_innerRadius > 0) {
        cv::ellipse(image, m_center, cv::Size(m_innerRadius, m_innerRadius), 0,
            m_startAngle, m_endAngle, color, thickness);
    }

    // 绘制两条半径线
    float startRad = m_startAngle * CV_PI / 180.0f;
    float endRad = m_endAngle * CV_PI / 180.0f;

    cv::Point outerStart(m_center.x + m_outerRadius * std::cos(startRad),
        m_center.y + m_outerRadius * std::sin(startRad));
    cv::Point outerEnd(m_center.x + m_outerRadius * std::cos(endRad),
        m_center.y + m_outerRadius * std::sin(endRad));

    // 如果有内半径，绘制内半径线
    if (m_innerRadius > 0) {
        cv::Point innerStart(m_center.x + m_innerRadius * std::cos(startRad),
            m_center.y + m_innerRadius * std::sin(startRad));
        cv::Point innerEnd(m_center.x + m_innerRadius * std::cos(endRad),
            m_center.y + m_innerRadius * std::sin(endRad));

        cv::line(image, innerStart, outerStart, color, thickness);
        cv::line(image, innerEnd, outerEnd, color, thickness);
    }
    else {
        cv::line(image, m_center, outerStart, color, thickness);
        cv::line(image, m_center, outerEnd, color, thickness);
    }
}

// 绘制填充扇形/圆环扇形
void CircularSector::DrawFilled(cv::Mat& image, const cv::Scalar& fillColor, const cv::Scalar& borderColor, int borderThickness) const {
    if (image.empty()) return;

    // 创建掩码图像
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);

    if (m_innerRadius == 0) {
        // 普通扇形
        std::vector<cv::Point> points;
        points.push_back(m_center);

        // 添加外圆弧上的点
        for (float angle = m_startAngle; angle <= m_endAngle; angle += 1.0f) {
            float rad = angle * CV_PI / 180.0f;
            cv::Point pt(m_center.x + m_outerRadius * std::cos(rad),
                m_center.y + m_outerRadius * std::sin(rad));
            points.push_back(pt);
        }
        points.push_back(m_center);

        cv::fillConvexPoly(mask, points, cv::Scalar(255));
    }
    else {
        // 圆环扇形：先填充大扇形，然后减去小扇形
        cv::Mat outerMask = cv::Mat::zeros(image.size(), CV_8UC1);
        cv::Mat innerMask = cv::Mat::zeros(image.size(), CV_8UC1);

        // 填充外扇形
        std::vector<cv::Point> outerPoints;
        outerPoints.push_back(m_center);
        for (float angle = m_startAngle; angle <= m_endAngle; angle += 1.0f) {
            float rad = angle * CV_PI / 180.0f;
            cv::Point pt(m_center.x + m_outerRadius * std::cos(rad),
                m_center.y + m_outerRadius * std::sin(rad));
            outerPoints.push_back(pt);
        }
        outerPoints.push_back(m_center);
        cv::fillConvexPoly(outerMask, outerPoints, cv::Scalar(255));

        // 填充内扇形（要减去的部分）
        std::vector<cv::Point> innerPoints;
        innerPoints.push_back(m_center);
        for (float angle = m_startAngle; angle <= m_endAngle; angle += 1.0f) {
            float rad = angle * CV_PI / 180.0f;
            cv::Point pt(m_center.x + m_innerRadius * std::cos(rad),
                m_center.y + m_innerRadius * std::sin(rad));
            innerPoints.push_back(pt);
        }
        innerPoints.push_back(m_center);
        cv::fillConvexPoly(innerMask, innerPoints, cv::Scalar(255));

        // 外扇形减去内扇形
        cv::subtract(outerMask, innerMask, mask);
    }

    // 应用填充颜色
    image.setTo(fillColor, mask);

    // 绘制边框
    DrawBorder(image, borderColor, borderThickness);
}

// 判断点是否在扇形/圆环扇形区域内
bool CircularSector::IsPointInRegion(const cv::Point& point) const {
    // 计算点到圆心的距离
    float distance = std::sqrt(std::pow(point.x - m_center.x, 2) +
        std::pow(point.y - m_center.y, 2));

    // 检查距离范围
    if (distance > m_outerRadius || distance < m_innerRadius) {
        return false;
    }

    // 计算点的角度
    float angle = std::atan2(point.y - m_center.y, point.x - m_center.x) * 180.0f / CV_PI;

    // 调整角度到0-360范围
    if (angle < 0) angle += 360.0f;

    // 处理角度跨越0度的情况
    if (m_startAngle > m_endAngle) {
        // 扇形跨越0度
        return (angle >= m_startAngle && angle <= 360.0f) ||
            (angle >= 0.0f && angle <= m_endAngle);
    }
    else {
        // 正常扇形
        return (angle >= m_startAngle && angle <= m_endAngle);
    }
}

// 计算扇形角度
float CircularSector::GetSectorAngle() const {
    if (m_startAngle > m_endAngle) {
        return (360.0f - m_startAngle) + m_endAngle;
    }
    return m_endAngle - m_startAngle;
}

// 计算扇形面积
float CircularSector::CalculateArea() const {
    float angleRad = GetSectorAngle() * CV_PI / 180.0f;
    float outerArea = 0.5f * angleRad * m_outerRadius * m_outerRadius;

    if (m_innerRadius > 0) {
        float innerArea = 0.5f * angleRad * m_innerRadius * m_innerRadius;
        return outerArea - innerArea;
    }

    return outerArea;
}



RotatedRectangle::RotatedRectangle(cv::Point center, cv::Size size, int radius, float angle)
    : m_center(center)
    , m_radius(radius)
    , m_angle(angle)
{
    CreateRotatedRect(size);
}

RotatedRectangle::~RotatedRectangle()
{
}


void RotatedRectangle::CreateRotatedRect(cv::Size size)
{
    if (m_radius <= 0) {
        m_rotatedRect = cv::RotatedRect(cv::Point2f(m_center.x, m_center.y), cv::Size2f(size.width, size.height), m_angle);
    }
    else {
        float radian = DegreesToRadians(m_angle);
        cv::Size2f rectSize(size.width, size.height);//矩形大小
        cv::Point2f roundCenter(m_center.x, m_center.y);//环绕中心点
        cv::Point2f rectCenter(roundCenter.x + m_radius * cos(radian), roundCenter.y + m_radius * sin(radian));//矩形的中心点

        // 计算旋转角度（使上下边与中心连线垂直）
        cv::Point2f direction = rectCenter - roundCenter;
        float angle = atan2(direction.y, direction.x) * 180.0f / CV_PI + 90.0f;
        m_rotatedRect = cv::RotatedRect(rectCenter, rectSize, angle);
    }
}

// 绘制边框
void RotatedRectangle::DrawBorder(cv::Mat& image, const cv::Scalar& color, int thickness) const
{
    cv::Point2f vertices[4];
    m_rotatedRect.points(vertices);
    for (int i = 0; i < 4; i++) {
        line(image, vertices[i], vertices[(i + 1) % 4], color, thickness);
    }
}

// 绘制填充
void RotatedRectangle::DrawFilled(cv::Mat& image, const cv::Scalar& fillColor, const cv::Scalar& borderColor, int borderThickness) const
{
    // to do
}

// 判断点是否在区域内
bool RotatedRectangle::IsPointInRegion(const cv::Point& point) const
{
    // 获取旋转矩形的角度（弧度）
    float angle = m_rotatedRect.angle * CV_PI / 180.0f;

    // 计算旋转矩阵
    float cos_angle = std::cos(-angle);
    float sin_angle = std::sin(-angle);

    // 将点转换到旋转矩形的局部坐标系
    cv::Point2f translated(point.x - m_rotatedRect.center.x, point.y - m_rotatedRect.center.y);

    // 旋转点（反向旋转）
    float local_x = translated.x * cos_angle - translated.y * sin_angle;
    float local_y = translated.x * sin_angle + translated.y * cos_angle;

    // 检查点是否在非旋转的矩形内
    float half_width = m_rotatedRect.size.width / 2.0f;
    float half_height = m_rotatedRect.size.height / 2.0f;

    return (std::abs(local_x) <= half_width) && (std::abs(local_y) <= half_height);
}
