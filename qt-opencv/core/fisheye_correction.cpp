#include "fisheye_correction.h"
#include "cv_utils.h"

UnwrapCircular::UnwrapCircular()
{
}

UnwrapCircular::~UnwrapCircular()
{
}

void UnwrapCircular::SetCropRect(CropRect rect)
{
    m_cropRect = rect;
}

CropRect UnwrapCircular::GetCropRect()
{
    return m_cropRect;
}

cv::Mat UnwrapCircular::GetUnwrapImage(const cv::Mat& src, int radius, bool isCrop)
{
    int R = (radius > 0) ? radius : std::min(src.cols, src.rows) / 2;
    cv::Point center(src.cols / 2, src.rows / 2);
    if (m_center != center || m_radius != R) {
        CreateMappingMatrix(center, R);
    }

    cv::Mat dstImage;
    remap(src, dstImage, m_mapX, m_mapY, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    if (isCrop) {
        //剪裁边缘数据，因为边缘的数据畸变非常严重
        return dstImage(cv::Rect(m_cropRect.left, m_cropRect.top, dstImage.cols - m_cropRect.right - m_cropRect.left, dstImage.rows - m_cropRect.bottom - m_cropRect.top));
    }
    return dstImage;
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


StretchCircular::StretchCircular()
{
    cv::Rect rc;

}

StretchCircular::~StretchCircular()
{
}

void StretchCircular::SetCropRect(CropRect rect)
{
    m_cropRect = rect;
}

CropRect StretchCircular::GetCropRect()
{
    return m_cropRect;
}

cv::Mat StretchCircular::GetStretchImage(const cv::Mat& src, int radius, bool isCrop)
{
    int R = (radius > 0) ? radius : std::min(src.cols, src.rows) / 2;
    cv::Point center(src.cols / 2, src.rows / 2);
    if (m_center != center || m_radius != R) {
        CreateMappingMatrix(center, R, 0);
    }

    cv::Mat dstImage;
    remap(src, dstImage, m_mapX, m_mapY, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    if (isCrop) {
        //剪裁边缘数据，因为边缘的数据畸变非常严重
        return dstImage(cv::Rect(m_cropRect.left, m_cropRect.top, dstImage.cols - m_cropRect.right - m_cropRect.left, dstImage.rows - m_cropRect.bottom - m_cropRect.top));
    }
    return dstImage;
}

void StretchCircular::CreateMappingMatrix(const cv::Point& center, int radius, int stretchMode)
{
    m_center = center;
    m_radius = radius;

    int width = radius * 2;
    int height = radius * 2;
    // 创建映射矩阵
    //cv::Mat map_x = cv::Mat::zeros(cv::Size(width, height), CV_32F);
    //cv::Mat map_y = cv::Mat::zeros(cv::Size(width, height), CV_32F);
    cv::Mat map_x(height, width, CV_32F);
    cv::Mat map_y(height, width, CV_32F);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = x - center.x;
            float dy = y - center.y;
            float distance = sqrt(dx * dx + dy * dy); // 当前点到中心的距离

            auto func = [](float x) ->float {
                //return x;
                return std::pow(x, 0.8f);
                //return std::sin(CV_PI / 2 * x);
                //return std::sqrt(1 - (x - 1) * (x - 1));
                //return std::asin(x) * 2 / CV_PI;
                //return -std::cos(CV_PI / 2 * (x + 1.0f));
                //return -std::acos(-x) / (CV_PI / 2) - 1.0f;
                };

            if (stretchMode == 0) {
                float scale = std::sqrt(radius * radius - dy * dy) / center.x;
                map_x.at<float>(y, x) = center.x + func(scale) * dx;
                map_y.at<float>(y, x) = y;
            }
            else if (stretchMode == 1) {
                float scale = std::sqrt(radius * radius - dx * dx) / center.y;
                map_x.at<float>(y, x) = x;
                map_y.at<float>(y, x) = center.y + func(scale) * dy;
            }
            else if (stretchMode == 2) {
                float theta = std::atan2(dy, dx); // 当前点的角度
                // 计算沿当前点方向上与矩形边交叉的点到中心的距离
                double cos_val = std::abs(std::cos(theta));
                double sin_val = std::abs(std::sin(theta));
                // 距离公式：r = a / max(|cosθ|, |sinθ|)
                float rectDistance = radius / std::max(cos_val, sin_val);
                float scale = distance / rectDistance;

                map_x.at<float>(y, x) = center.x + func(scale) * radius * cos(theta);
                map_y.at<float>(y, x) = center.x + func(scale) * radius * sin(theta);
            }
            else if (stretchMode == 3) {
                // 如果点在鱼眼半径内
                if (distance <= radius) {
                    float theta = std::atan2(dy, dx); // 当前点的角度
                    float scale = distance / radius;
                    map_x.at<float>(y, x) = center.x + func(scale) * radius * cos(theta);
                    map_y.at<float>(y, x) = center.y + func(scale) * radius * sin(theta);
                }
                else {
                    // 保持原样
                    map_x.at<float>(y, x) = x;
                    map_y.at<float>(y, x) = y;
                }
            }
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
