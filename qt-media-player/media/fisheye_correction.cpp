#include "fisheye_correction.h"

// 角度转弧度函数
double DegreesToRadians(double degrees) {
    return degrees * (CV_PI / 180.0);
}

// 弧度转角度函数  
double RadiansToDegrees(double radians) {
    return radians * (180.0 / CV_PI);
}

// 计算扇形角度（度）
double GetSectorAngle(double arc, double radius) {
    if (radius == 0) {
        return 0;
    }
    return (arc * 180.0) / (CV_PI * radius);
}

// 计算扇形角度（弧度）
double GetSectorAngleRadians(double arc, double radius) {
    if (radius == 0) {
        return 0;
    }
    return arc / radius;
}

cv::Mat GetLR2TBCroppingImage(const cv::Mat& src)
{
    int height = src.rows;
    int width = src.cols;
    int midX = width / 2;

    cv::Mat leftPart = src(cv::Rect(0, 0, midX, height));
    cv::Mat rightPart = src(cv::Rect(midX, 0, midX, height));
    cv::Mat result(height * 2, midX, src.type());

    // 将左右部分复制到结果图像的相应位置
    leftPart.copyTo(result(cv::Rect(0, 0, midX, height)));
    rightPart.copyTo(result(cv::Rect(0, height, midX, height)));
    return result;
}

void UnwrapCircular::SetCropRows(int cropRows)
{
    m_cropRows = cropRows;
}

int UnwrapCircular::GetCropRows()
{
    return m_cropRows;
}

cv::Mat UnwrapCircular::GetUnwrapImage(const cv::Mat& src, int radius, bool isCropRows)
{
    int R = (radius > 0) ? radius : std::min(src.cols, src.rows) / 2;
    cv::Point center(src.cols / 2, src.rows / 2);
    if (m_center != center || m_radius != R) {
        CreateMappingMatrix(center, R);
    }

    cv::Mat unwrappedImage;
    remap(src, unwrappedImage, m_mapX, m_mapY, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    if (isCropRows && m_cropRows > 0 && m_cropRows < unwrappedImage.rows) {
        return unwrappedImage(cv::Rect(0, m_cropRows, unwrappedImage.cols, unwrappedImage.rows - m_cropRows));//移除顶部的数据，因为顶部的数据畸变非常严重
    }
    return unwrappedImage;
}

cv::Point2f UnwrapCircular::GetOriginPoint(const cv::Point2f& point)
{
    int rectWidth = round(2 * CV_PI * m_radius);
    double theta = (2 * CV_PI * point.x) / rectWidth + CV_PI * 0.5f;//这里增加90度是为了保持和海康的画面一致，后续可以考虑不需要
    //double theta = (2 * CV_PI * point.x) / rectWidth;
    double r = point.y;

    float x = m_center.x - r * cos(theta);//这里使用+还是-需要和圆形展开保持一致
    float y = m_center.y + r * sin(theta);
    return cv::Point2f(x, y);
}

void UnwrapCircular::CreateMappingMatrix(const cv::Point& center, int radius)
{
    m_center = center;
    m_radius = radius;
    int rectWidth = round(2 * CV_PI * radius);
    int rectHeight = radius;

    // 创建映射矩阵
    cv::Mat map_x(rectHeight, rectWidth, CV_32F);
    cv::Mat map_y(rectHeight, rectWidth, CV_32F);

    for (int y = 0; y < rectHeight; y++) {
        for (int x = 0; x < rectWidth; x++) {
            double theta = (2 * CV_PI * x) / rectWidth + CV_PI * 0.5f;//这里增加90度是为了保持和海康的画面一致，后续可以考虑不需要
            //double theta = (2 * CV_PI * x) / rectWidth;
            double r = y;
            //map_x.at<float>(y, x) = center.x + r * cos(theta);//顺时针展开
            map_x.at<float>(y, x) = center.x - r * cos(theta);//逆时针展开更符合视觉效果
            map_y.at<float>(y, x) = center.y + r * sin(theta);
        }
    }
    m_mapX = map_x;
    m_mapY = map_y;
}

FishEyeCorrection::FishEyeCorrection()
    : pUnwrapCircular(new UnwrapCircular())
{
}

FishEyeCorrection::~FishEyeCorrection()
{
    delete pUnwrapCircular;
}

UnwrapCircular* FishEyeCorrection::GetUnwrapCircular()
{
    return pUnwrapCircular;
}


cv::Mat FishEyeCorrection::ExtractRotatedRegionROI(const cv::Mat& src, const cv::RotatedRect& rotatedRect)
{
    // 获取旋转矩形的最小外接矩形
    cv::Rect boundingRect = rotatedRect.boundingRect();

    // 扩展边界以确保包含整个旋转矩形
    int padding = 50;
    boundingRect.x = std::max(0, boundingRect.x - padding);
    boundingRect.y = std::max(0, boundingRect.y - padding);
    boundingRect.width = std::min(src.cols - boundingRect.x, boundingRect.width + 2 * padding);
    boundingRect.height = std::min(src.rows - boundingRect.y, boundingRect.height + 2 * padding);

    // 提取ROI
    cv::Mat roi = src(boundingRect).clone();

    // 调整旋转中心坐标为ROI内的相对坐标
    cv::Point2f adjustedCenter = rotatedRect.center - cv::Point2f(boundingRect.x, boundingRect.y);

    // 只旋转ROI区域，提高效率
    cv::Mat rotationMatrix = getRotationMatrix2D(adjustedCenter, rotatedRect.angle, 1.0);
    cv::Mat rotatedROI;
    warpAffine(roi, rotatedROI, rotationMatrix, roi.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    // 计算在旋转后图像中的裁剪位置
    cv::Rect extractRect(adjustedCenter.x - rotatedRect.size.width / 2,
        adjustedCenter.y - rotatedRect.size.height / 2,
        rotatedRect.size.width, rotatedRect.size.height);

    extractRect = extractRect & cv::Rect(0, 0, rotatedROI.cols, rotatedROI.rows);

    if (extractRect.width > 0 && extractRect.height > 0) {
        return rotatedROI(extractRect).clone();
    }
    return cv::Mat();
}


bool FishEyeCorrection::IsPointInRotatedRectTransform(const cv::Point2f& point, const cv::RotatedRect& rect)
{
    // 获取旋转矩形的角度（弧度）
    float angle = rect.angle * CV_PI / 180.0f;

    // 计算旋转矩阵
    float cos_angle = std::cos(-angle);
    float sin_angle = std::sin(-angle);

    // 将点转换到旋转矩形的局部坐标系
    cv::Point2f translated(point.x - rect.center.x, point.y - rect.center.y);

    // 旋转点（反向旋转）
    float local_x = translated.x * cos_angle - translated.y * sin_angle;
    float local_y = translated.x * sin_angle + translated.y * cos_angle;

    // 检查点是否在非旋转的矩形内
    float half_width = rect.size.width / 2.0f;
    float half_height = rect.size.height / 2.0f;

    return (std::abs(local_x) <= half_width) && (std::abs(local_y) <= half_height);
}

// 将圆形图像展开成矩形
cv::Mat UnwrapCircularImage(const cv::Mat& circularImage, int radius = -1)
{
    // 确定圆的半径（如果未指定，使用图像的最小边长的一半）
    int R = (radius > 0) ? radius : std::min(circularImage.cols, circularImage.rows) / 2;

    // 圆心坐标
    cv::Point center(circularImage.cols / 2, circularImage.rows / 2);

    // 创建展开后的矩形图像：宽度为圆周长 2πR，高度为半径 R
    int rectWidth = round(2 * CV_PI * R);
    int rectHeight = R;
    cv::Mat unwrappedImage(rectHeight, rectWidth, circularImage.type(), cv::Scalar(0));

    // 遍历矩形图像的每个像素
    for (int y = 0; y < rectHeight; y++) {
        for (int x = 0; x < rectWidth; x++) {
            // 将矩形坐标转换为极坐标
            double theta = (2 * CV_PI * x) / rectWidth;  // 角度 [0, 2π]
            double r = y;                                // 半径 [0, R]

            // 将极坐标转换为原图像中的笛卡尔坐标
            int srcX = center.x - r * cos(theta);
            int srcY = center.y + r * sin(theta);

            // 确保坐标在原始图像范围内
            if (srcX >= 0 && srcX < circularImage.cols &&
                srcY >= 0 && srcY < circularImage.rows) {
                unwrappedImage.at<cv::Vec3b>(y, x) = circularImage.at<cv::Vec3b>(srcY, srcX);
            }
        }
    }
    return unwrappedImage;
}

cv::Mat UnwrapCircularImageOptimized(const cv::Mat& circularImage, int radius = -1)
{
    int R = (radius > 0) ? radius : std::min(circularImage.cols, circularImage.rows) / 2;
    cv::Point center(circularImage.cols / 2, circularImage.rows / 2);

    int rectWidth = round(2 * CV_PI * R);
    int rectHeight = R;

    // 创建映射矩阵
    cv::Mat map_x(rectHeight, rectWidth, CV_32F);
    cv::Mat map_y(rectHeight, rectWidth, CV_32F);

    for (int y = 0; y < rectHeight; y++) {
        for (int x = 0; x < rectWidth; x++) {
            double theta = (2 * CV_PI * x) / rectWidth;
            double r = y;

            map_x.at<float>(y, x) = center.x + r * cos(theta);
            map_y.at<float>(y, x) = center.y + r * sin(theta);
        }
    }

    cv::Mat unwrappedImage;
    remap(circularImage, unwrappedImage, map_x, map_y, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    return unwrappedImage;
}
