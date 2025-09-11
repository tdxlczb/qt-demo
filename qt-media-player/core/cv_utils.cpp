#include "cv_utils.h"

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

cv::Point GetSymmetricPoint(const cv::Point& src, const cv::Point& center)
{
    float symmetricX = 2 * center.x - src.x;
    float symmetricY = 2 * center.y - src.y;
    return cv::Point(symmetricX, symmetricY);
}