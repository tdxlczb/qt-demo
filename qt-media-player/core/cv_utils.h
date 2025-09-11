#ifndef CV_UTILS_H
#define CV_UTILS_H

#include <opencv2/opencv.hpp>

// 角度转弧度函数
double DegreesToRadians(double degrees);

// 弧度转角度函数  
double RadiansToDegrees(double radians);

// 计算扇形角度（度）
double GetSectorAngle(double arc, double radius);

// 计算扇形角度（弧度）
double GetSectorAngleRadians(double arc, double radius);

//图片左右裁剪成上下
cv::Mat GetLR2TBCroppingImage(const cv::Mat& src);

//获取中心点对称坐标
cv::Point GetSymmetricPoint(const cv::Point& src, const cv::Point& center);


#endif // CV_UTILS_H
