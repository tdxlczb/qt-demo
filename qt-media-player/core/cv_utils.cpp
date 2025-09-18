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
            double radius = y;                           // 半径 [0, R]

            // 将极坐标转换为原图像中的笛卡尔坐标
            int srcX = center.x + radius * cos(theta);
            int srcY = center.y + radius * sin(theta);

            // 确保坐标在原始图像范围内
            if (srcX >= 0 && srcX < circularImage.cols &&
                srcY >= 0 && srcY < circularImage.rows) {
                unwrappedImage.at<cv::Vec3b>(y, x) = circularImage.at<cv::Vec3b>(srcY, srcX);
            }
        }
    }
    return unwrappedImage;
}

//优化使用remap创建映射矩阵实现，不使用使用映射矩阵，每次计算耗时较高，不能用于处理视频图像
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
            double radius = y;

            map_x.at<float>(y, x) = center.x + radius * cos(theta);
            map_y.at<float>(y, x) = center.y + radius * sin(theta);
        }
    }

    cv::Mat unwrappedImage;
    remap(circularImage, unwrappedImage, map_x, map_y, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    return unwrappedImage;
}

// 反向操作：将矩形图像重新卷成圆形
cv::Mat RewrapToCircularImage(const cv::Mat& unwrappedImage, cv::Size circularSize) 
{
    cv::Mat circularImage(circularSize, unwrappedImage.type(), cv::Scalar(0));

    int R = unwrappedImage.rows;  // 矩形高度就是半径
    cv::Point center(circularSize.width / 2, circularSize.height / 2);
    int rectWidth = unwrappedImage.cols;

    // 遍历圆形图像的每个像素
    for (int y = 0; y < circularSize.height; y++) {
        for (int x = 0; x < circularSize.width; x++) {
            // 计算当前点到圆心的距离和角度
            double dx = x - center.x;
            double dy = y - center.y;
            double r = sqrt(dx * dx + dy * dy);
            double theta = atan2(dy, dx);
            if (theta < 0) theta += 2 * CV_PI;  // 将角度转换到 [0, 2π]

            // 如果点在圆内
            if (r <= R) {
                // 将极坐标转换为矩形坐标
                int rectX = round((theta * rectWidth) / (2 * CV_PI));
                int rectY = round(r);

                // 确保坐标在矩形图像范围内
                if (rectX >= 0 && rectX < rectWidth && rectY >= 0 && rectY < R) {
                    circularImage.at<cv::Vec3b>(y, x) = unwrappedImage.at<cv::Vec3b>(rectY, rectX);
                }
            }
        }
    }

    return circularImage;
}