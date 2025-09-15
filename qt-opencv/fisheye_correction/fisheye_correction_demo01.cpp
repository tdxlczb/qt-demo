#include "fisheye_correction_demo01.h"

#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
using namespace std;
using namespace cv;


#include "core/graphics.h"
#include "core/cv_utils.h"
#include "core/fisheye_correction.h"

int UnwrapCircularDemo()
{
    // 读取圆形图像（或创建测试图像）
    Mat circularImage = imread(R"(E:\code\media\image\Snipaste_2025-09-05_10-51-22.jpg)");
    int rect_width = 200;  // 矩形宽度
    int rect_height = 200;  // 矩形高度
    cv::Point top_left(circularImage.cols / 2 - rect_width / 2, 100); // 左上角坐标
    //cv::Point top_left(circularImage.cols / 2 - rect_width / 2, circularImage.rows / 2 - rect_height / 2); // 左上角坐标
    cv::Point bottom_right(top_left.x + rect_width, top_left.y + rect_height);         // 右下角坐标

    //绘制红色矩形（BGR格式：(0, 0, 255)）
    //cv::rectangle(circularImage, top_left, bottom_right, cv::Scalar(0, 0, 255), 2); // 线宽=2

    int radius = min(circularImage.cols, circularImage.rows) / 2;
    cv::resize(circularImage, circularImage, cv::Size(radius * 2, radius * 2));

    cv::Point center(circularImage.cols / 2, circularImage.rows / 2);

    RotatedRectangle rotatedRc(center, cv::Size(200, 100), 100, 10.0f);
    //rotatedRc.DrawBorder(circularImage, cv::Scalar(0, 0, 255), 2);

    CircularSector sector(center, 90.0f, 120.0f, 200, 100);
    sector.DrawBorder(circularImage, cv::Scalar(0, 0, 255), 2);

    // 展开圆形图像
    Mat unwrapped = UnwrapCircularImageOptimized(circularImage, radius);
    //unwrapped = unwrapped(Rect(0, 60, unwrapped.cols, unwrapped.rows - 60));//裁剪顶部畸变严重的部分
    // 显示结果
    imshow("原始圆形图像", circularImage);
    imshow("展开后的矩形图像", unwrapped);

    //// 可选：重新卷回圆形（验证算法正确性）
    //Mat rewrapped = rewrapToCircularImage(unwrapped, circularImage.size());
    //imshow("重新卷回的圆形图像", rewrapped);

    waitKey(0);
    return 0;
}

int StretchCircularDemo() {
    cv::Mat src = cv::imread(R"(E:\code\media\image\Snipaste_2025-09-13_11-52-55.jpg)");
    if (src.empty()) {
        std::cerr << "Error: Could not load image!" << std::endl;
        return -1;
    }
    float maxRadius = std::min(src.cols / 2.0f, src.rows / 2.0f);
    cv::resize(src, src, cv::Size(2 * maxRadius, 2 * maxRadius));
    cv::Point center(src.cols / 2, src.rows / 2);

    RotatedRectangle rotatedRc(center, cv::Size(100, 100), 350, 130.0f);
    rotatedRc.DrawBorder(src, cv::Scalar(0, 0, 255), 2);

    CircularSector sector(center, 120.0f, 150.0f, 500, 300);
    sector.DrawBorder(src, cv::Scalar(0, 0, 255), 2);

    StretchCircular sc;
    //sc.SetCropRect({ 50,100,50,100 });
    cv::Mat dst = sc.GetStretchImage(src, maxRadius, false);

    cv::imshow("Original", src);
    cv::imshow("Fisheye", dst);
    cv::waitKey(0);
    return 0;
}

int FisheyeDemo01() {

    // 相机内参矩阵（根据自己的相机参数修改）
    cv::Mat K = (cv::Mat_<double>(3, 3) << 286.1809997558594, 0.0, 421.6383972167969,
        0.0, 286.3576965332031, 403.9013977050781,
        0.0, 0.0, 1.0);
    // 畸变系数 - 假设有4个: k1, k2, k3, k4，（根据自己的相机参数修改）
    cv::Mat D = (cv::Mat_<double>(4, 1) << -0.008326118811964989, 0.04620290920138359, -0.04403631016612053, 0.00837636087089777);

    //cv::Matx33d K(500.0, 0.0, 320.0,
    //    0.0, 500.0, 240.0,
    //    0.0, 0.0, 1.0);

    //cv::Vec4d D(-0.1, 0.05, -0.001, 0.0005);

    cv::Mat image = cv::imread(R"(E:\code\media\image\Snipaste_2025-09-10_20-18-10.jpg)");
    cv::Size imageSize(image.cols, image.rows);
    cv::Mat undistortImg;
    // 计算最优新相机内参矩阵
    cv::Mat newK = cv::getOptimalNewCameraMatrix(K, D, imageSize, 1, imageSize, 0);
    // 方法一：使用 estimateNewCameraMatrixForUndistortRectify
    cv::Matx33d new_K;
    cv::Rect roi;

    double balance = 0.8; // 在0-1之间调整这个值

    // 正确的调用方式：new_K 和 roi 是输出参数
    cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
        K, D, imageSize, cv::Matx33d::eye(),
        new_K, balance, imageSize
    );

    //注意，如果是处理视频，建议将该函数内部实现抽离，映射矩阵只需要生成一次即可，可以优化效率
    //cv::fisheye::undistortImage(image, undistortImg, K, D, newK);

    RotatedRectangle rotatedRc(cv::Point(image.cols / 2, image.rows / 2), cv::Size(200, 100), 100, 10.0f);
    rotatedRc.DrawBorder(image, cv::Scalar(0, 0, 255), 2);

    Mat map1, map2;
    cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat(), K, imageSize, CV_16SC2, map1, map2);
    remap(image, undistortImg, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    cv::imshow("Original", image);
    cv::imshow("Fisheye", undistortImg);
    cv::waitKey(0);
    return 0;
}

#include <opencv2/opencv.hpp>
#include <cmath>

int FisheyeDemo02() {
    return 0;
}

int FisheyeDemo03() {
    return 0;
}

int FisheyeDemo04() {
    cv::Mat src = cv::imread(R"(E:\code\media\image\grid.jpg)");
    if (src.empty()) {
        std::cerr << "Error: Could not load image!" << std::endl;
        return -1;
    }
    float maxRadius = std::min(src.cols / 2.0f, src.rows / 2.0f);
    cv::resize(src, src, cv::Size(2 * maxRadius, 2 * maxRadius));
    cv::Point center(src.cols / 2.0f, src.rows / 2.0f);
    CircularSector sector(center, 60.0f, 120.0f, 200, 100);
    sector.DrawBorder(src, cv::Scalar(0, 0, 255), 2);

    imshow("Original Image", src);
    //imshow("Fisheye Effect", fisheyeImage);
    waitKey(0);
    return 0;
}
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

// 鱼眼畸变参数结构体
struct FisheyeParams {
    double k1; // 径向畸变系数
    double k2;
    double k3;
    double k4;
    double cx, cy; // 畸变中心（通常为图像中心）
    double fx, fy; // 焦距
};

// 应用鱼眼畸变效果
Mat applyFisheyeDistortion(const Mat& inputImage, const FisheyeParams& params) {
    Mat distortedImage = Mat::zeros(inputImage.size(), inputImage.type());

    int width = inputImage.cols;
    int height = inputImage.rows;

    // 如果未指定畸变中心，使用图像中心
    double cx = params.cx > 0 ? params.cx : width / 2.0;
    double cy = params.cy > 0 ? params.cy : height / 2.0;

    // 遍历目标图像的每个像素
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 将像素坐标转换为归一化相机坐标
            double x_norm = (x - cx) / params.fx;
            double y_norm = (y - cy) / params.fy;

            // 计算径向距离
            double r = sqrt(x_norm * x_norm + y_norm * y_norm);

            if (r == 0) {
                // 中心点不需要畸变
                distortedImage.at<Vec3b>(y, x) = inputImage.at<Vec3b>(y, x);
                continue;
            }

            // 计算畸变后的角度（鱼眼模型）
            double theta = atan(r);
            double theta_d = theta * (1 + params.k1 * pow(theta, 2) +
                params.k2 * pow(theta, 4) +
                params.k3 * pow(theta, 6) +
                params.k4 * pow(theta, 8));

            // 计算畸变后的归一化坐标
            double scale = theta_d / r;
            double x_dist_norm = x_norm * scale;
            double y_dist_norm = y_norm * scale;

            // 将归一化坐标转换回像素坐标
            int x_dist = round(x_dist_norm * params.fx + cx);
            int y_dist = round(y_dist_norm * params.fy + cy);

            // 如果坐标在原始图像范围内，复制像素值
            if (x_dist >= 0 && x_dist < width && y_dist >= 0 && y_dist < height) {
                distortedImage.at<Vec3b>(y, x) = inputImage.at<Vec3b>(y_dist, x_dist);
            }
        }
    }

    return distortedImage;
}

// 简化的鱼眼畸变效果（使用预设参数）
Mat applySimpleFisheye(const Mat& inputImage, double strength = 1.0) {
    Mat distortedImage = Mat::zeros(inputImage.size(), inputImage.type());

    int width = inputImage.cols;
    int height = inputImage.rows;
    double cx = width / 2.0;
    double cy = height / 2.0;

    // 简化的鱼眼模型参数
    double max_radius = sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算当前点到中心的距离
            double dx = x - cx;
            double dy = y - cy;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance == 0) {
                distortedImage.at<Vec3b>(y, x) = inputImage.at<Vec3b>(y, x);
                continue;
            }

            // 计算畸变因子（非线性函数）
            double r = distance / max_radius;
            double distortion = 1.0 - strength * r * r;

            // 应用畸变
            int src_x = static_cast<int>(cx + dx * distortion);
            int src_y = static_cast<int>(cy + dy * distortion);

            // 确保坐标在图像范围内
            src_x = max(0, min(width - 1, src_x));
            src_y = max(0, min(height - 1, src_y));

            distortedImage.at<Vec3b>(y, x) = inputImage.at<Vec3b>(src_y, src_x);
        }
    }

    return distortedImage;
}

int FisheyeDemo05() {

    const cv::Size imageSize(1280, 800);

    const cv::Matx33d K(558.478087865323, 0, 620.458515360843,
        0, 560.506767351568, 381.939424848348,
        0, 0, 1);

    const cv::Vec4d D(-0.0014613319981768, -0.00329861110580401, 0.00605760088590183, -0.00374209380722371);


    const cv::Matx33d R(9.9756700084424932e-01, 6.9698277640183867e-02, 1.4929569991321144e-03,
        -6.9711825162322980e-02, 9.9748249845531767e-01, 1.2997180766418455e-02,
        -5.8331736398316541e-04, -1.3069635393884985e-02, 9.9991441852366736e-01);

    const cv::Vec3d T(-9.9217369356044638e-02, 3.1741831972356663e-03, 1.8551007952921010e-04);


    // we use it to reduce patch size for images in testdata
    auto throwAwayHalf = [](Mat img)
        {
            int whalf = img.cols / 2, hhalf = img.rows / 2;
            Rect tl(0, 0, whalf, hhalf), br(whalf, hhalf, whalf, hhalf);
            img(tl) = 0;
            img(br) = 0;
        };

    cv::Matx33d theK = K;
    cv::Mat theD = cv::Mat(D);
    cv::Matx33d newK = theK;
    cv::Mat distorted = imread(R"(E:\code\media\image\hediquanjingyuyan-22918431_3.jpg)");
    cv::Mat undistorted;
    {
        newK(0, 0) = 100;
        newK(1, 1) = 100;
        cv::fisheye::undistortImage(distorted, undistorted, theK, theD, newK);

        imshow("原始图像", distorted);
        imshow("鱼眼畸变效果", undistorted);
    }
    {
        double balance = 1.0;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(theK, theD, distorted.size(), cv::noArray(), newK, balance);
        cv::fisheye::undistortImage(distorted, undistorted, theK, theD, newK);


        imshow("原始图像", distorted);
        imshow("鱼眼畸变效果", undistorted);

    }

    {
        double balance = 0.0;
        cv::fisheye::estimateNewCameraMatrixForUndistortRectify(theK, theD, distorted.size(), cv::noArray(), newK, balance);
        cv::fisheye::undistortImage(distorted, undistorted, theK, theD, newK);

        imshow("原始图像", distorted);
        imshow("鱼眼畸变效果", undistorted);

    }
    waitKey(0);
    return 0;
}
