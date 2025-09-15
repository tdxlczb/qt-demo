#include "main_window.h"

#include <QApplication>
#include <opencv2/opencv.hpp>
#include "fisheye_correction/fisheye_correction_demo01.h"
#include "image_stitching/image_stitching_demo01.h"

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
//}

int main(int argc, char* argv[])
{
    //UnwrapCircularDemo();
    StretchCircularDemo();
    //FisheyeDemo02();
    //ImageStitchingDemo02();
    return 0;
}

#include <opencv2/opencv.hpp>
#include <iostream>

int main2() {
    // 读取图像
    cv::Mat img = cv::imread(R"(E:\code\media\image\Snipaste_2025-09-10_20-18-10.jpg)");
    if (img.empty()) {
        std::cout << "无法读取图像" << std::endl;
        return -1;
    }

    int width = img.cols;
    int height = img.rows;

    // 估算相机内参矩阵 K
    // 对于鱼眼镜头，焦距通常比普通镜头大
    double fx = width * 0.8;  // 比图像宽度稍大的焦距
    double fy = width * 0.8;  // 通常 fx ≈ fy
    double cx = width / 2.0;  // 图像中心
    double cy = height / 2.0; // 图像 中心

    cv::Matx33d K(fx, 0, cx,
        0, fy, cy,
        0, 0, 1);

    // 估算畸变系数 D [k1, k2, k3, k4]
    // 对于鱼眼镜头，k1 通常是较大的负值，k2、k3、k4 逐渐减小
    // 这样确保中心畸变小，边缘畸变大
    double k1 = -0.5;   // 主要径向畸变系数（负值产生桶形畸变）
    double k2 = 0.2;    // 次要径向畸变系数
    double k3 = -0.1;  // 三次径向畸变系数
    double k4 = 0.05;   // 四次径向畸变系数

    cv::Vec4d D(k1, k2, k3, k4);

    // 显示估算的参数
    std::cout << "估算的相机内参矩阵 K:" << std::endl;
    std::cout << "fx = " << fx << std::endl;
    std::cout << "fy = " << fy << std::endl;
    std::cout << "cx = " << cx << std::endl;
    std::cout << "cy = " << cy << std::endl;

    std::cout << "\n估算的畸变系数 D:" << std::endl;
    std::cout << "k1 = " << k1 << " (主要径向畸变)" << std::endl;
    std::cout << "k2 = " << k2 << " (次要径向畸变)" << std::endl;
    std::cout << "k3 = " << k3 << " (三次径向畸变)" << std::endl;
    std::cout << "k4 = " << k4 << " (四次径向畸变)" << std::endl;

    // 鱼眼校正
    cv::Matx33d new_K;
    double balance = 0.7;  // 平衡参数

    cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
        K, D, cv::Size(width, height), cv::Matx33d::eye(),
        new_K, balance, cv::Size(width, height)
    );

    cv::Mat undistorted_img;
    cv::fisheye::undistortImage(
        img, undistorted_img, K, D, K, cv::Size(width, height)
    );

    cv::imshow("原始鱼眼图像", img);
    cv::imshow("校正后图像", undistorted_img);
    cv::waitKey(0);

    return 0;
}