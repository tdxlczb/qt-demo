#include "main_window.h"

#include <QApplication>

int main0(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}


#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
using namespace std;
using namespace cv;

//#pragma comment(lib, "opencv_core430d.lib")
//#pragma comment(lib, "opencv_highgui430d.lib")
//#pragma comment(lib, "opencv_imgcodecs430d.lib")
//#pragma comment(lib, "opencv_imgprocd.lib")

int main(int argc, char *argv[])
{
    //读取图片
    Mat Src = imread(R"(E:\code\media\haikang_IPCamera 05_haikang_20250516143954_90301.bmp)");	//imshow("Src",Src);
    cout << Src.size() << endl;
    int nbottom = 0;
    int ntop = 0;
    int nright = 0;
    int nleft = 0;

    int rect_width = 200;  // 矩形宽度
    int rect_height = 100;  // 矩形高度
    cv::Point top_left(Src.cols / 2 - rect_width / 2, Src.rows / 2 - rect_height / 2); // 左上角坐标
    cv::Point bottom_right(top_left.x + rect_width, top_left.y + rect_height);         // 右下角坐标

    // 3. 绘制红色矩形（BGR格式：(0, 0, 255)）
    cv::rectangle(Src, top_left, bottom_right, cv::Scalar(0, 0, 255), 2); // 线宽=2

    //遍历寻找上边界
    int nflag = 0;
    for (int i = 0; i < Src.rows - 1; i++)
    {
        for (int j = 0; j < Src.cols - 1; j++)
        {
            uchar I = 0.59 * Src.at<Vec3b>(i, j)[0] + 0.11 * Src.at<Vec3b>(i, j)[1] + 0.3 * Src.at<Vec3b>(i, j)[2];
            if (I > 20)
            {
                I = 0.59 * Src.at<Vec3b>(i + 1, j)[0] + 0.11 * Src.at<Vec3b>(i + 1, j)[1] + 0.3 * Src.at<Vec3b>(i + 1, j)[2];
                if (I > 20)
                {
                    ntop = i;
                    nflag = 1;
                    break;
                }
            }
        }
        if (nflag == 1)
        {
            break;
        }
    }
    //遍历寻找下边界
    nflag = 0;
    for (int i = Src.rows - 1; i > 1; i--)
    {
        for (int j = 0; j < Src.cols - 1; j++)
        {
            uchar I = 0.59 * Src.at<Vec3b>(i, j)[0] + 0.11 * Src.at<Vec3b>(i, j)[1] + 0.3 * Src.at<Vec3b>(i, j)[2];
            if (I > 20)
            {
                I = 0.59 * Src.at<Vec3b>(i - 1, j)[0] + 0.11 * Src.at<Vec3b>(i - 1, j)[1] + 0.3 * Src.at<Vec3b>(i - 1, j)[2];
                if (I > 20)
                {
                    nbottom = i;
                    nflag = 1;
                    break;
                }
            }
        }
        if (nflag == 1)
        {
            break;
        }
    }
    //遍历寻找左边界
    nflag = 0;
    for (int j = 0; j < Src.cols - 1; j++)
    {
        for (int i = 0; i < Src.rows; i++)
        {
            uchar I = 0.59 * Src.at<Vec3b>(i, j)[0] + 0.11 * Src.at<Vec3b>(i, j)[1] + 0.3 * Src.at<Vec3b>(i, j)[2];
            if (I > 20)
            {
                I = 0.59 * Src.at<Vec3b>(i, j + 1)[0] + 0.11 * Src.at<Vec3b>(i, j + 1)[1] + 0.3 * Src.at<Vec3b>(i, j + 1)[2];
                if (I > 20)
                {
                    nleft = j;
                    nflag = 1;
                    break;
                }
            }
        }
        if (nflag == 1)
        {
            break;
        }
    }
    //遍历寻找右边界
    nflag = 0;
    for (int j = Src.cols - 1; j > 0; j--)
    {
        for (int i = 0; i < Src.rows; i++)
        {
            uchar I = 0.59 * Src.at<Vec3b>(i, j)[0] + 0.11 * Src.at<Vec3b>(i, j)[1] + 0.3 * Src.at<Vec3b>(i, j)[2];
            if (I > 20)
            {
                I = 0.59 * Src.at<Vec3b>(i, j - 1)[0] + 0.11 * Src.at<Vec3b>(i, j - 1)[1] + 0.3 * Src.at<Vec3b>(i, j - 1)[2];
                if (I > 20)
                {
                    nright = j;
                    nflag = 1;
                    break;
                }
            }
        }
        if (nflag == 1)
        {
            break;
        }
    }
    cout << ntop << endl;
    cout << nbottom << endl;
    cout << nleft << endl;
    cout << nright << endl;

    //根据边界值来获得直径
    int d = min(nright - nleft, nbottom - ntop);

    Mat imgRoi;
    imgRoi = Src(Rect(nleft, ntop, d, d));
    cv::imshow("imgRoi", imgRoi);

    Mat dst(imgRoi.size(), CV_8UC3, Scalar(255, 255, 255));

    //建立映射表
    Mat map_x, map_y;
    map_x.create(imgRoi.size(), CV_32FC1);
    map_y.create(imgRoi.size(), CV_32FC1);
    for (int j = 0; j < d - 1; j++)
    {
        for (int i = 0; i < d - 1; i++)
        {
            map_x.at<float>(i, j) = static_cast<float>(d / 2.0 + i / 2.0 * cos(1.0 * j / d * 2 * CV_PI));
            map_y.at<float>(i, j) = static_cast<float>(d / 2.0 + i / 2.0 * sin(1.0 * j / d * 2 * CV_PI));
        }
    }

    remap(imgRoi, dst, map_x, map_y, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
    //重设大小
    resize(dst, dst, Size(), 1.0, 0.5);

    cv::imshow("dst", dst);
    waitKey();
    return 0;
}

#include <opencv2/opencv.hpp>
#include <cmath>

int main3() {
    cv::Mat src = cv::imread(R"(E:\code\media\grid.jpg)");
    if (src.empty()) {
        std::cerr << "Error: Could not load image!" << std::endl;
        return -1;
    }

    // 创建一个极坐标映射
    cv::Mat dst(src.size(), src.type());
    cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    float maxRadius = std::min(center.x, center.y);

    for (int y = 0; y < dst.rows; y++) {
        for (int x = 0; x < dst.cols; x++) {
            // 计算当前点到中心的距离和角度
            float dx = x - center.x;
            float dy = y - center.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            // 归一化并应用非线性变换（模拟鱼眼）
            float normalizedDist = distance / maxRadius;
            if (normalizedDist > 1.0f) normalizedDist = 1.0f;

            // 调整畸变强度（0.5 是控制参数）
            float fisheyeDist = std::pow(normalizedDist, 0.5f) * maxRadius;

            // 计算原始图像中的对应点
            if (distance > 0) {
                float angle = std::atan2(dy, dx);
                float srcX = center.x + fisheyeDist * std::cos(angle);
                float srcY = center.y + fisheyeDist * std::sin(angle);

                // 双线性插值
                if (srcX >= 0 && srcX < src.cols - 1 && srcY >= 0 && srcY < src.rows - 1) {
                    dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(cv::Point(srcX, srcY));
                }
            } else {
                dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(center);
            }
        }
    }

    cv::imshow("Original", src);
    cv::imshow("Fake Fisheye", dst);
    cv::waitKey(0);
    //cv::imwrite("fake_fisheye.jpg", dst);

    return 0;
}


#include <opencv2/opencv.hpp>

int main4() {
    // 1. 加载图片
    cv::Mat img = cv::imread(R"(E:\code\media\grid.jpg)"); // 替换为你的图片路径
    if (img.empty()) {
        std::cerr << "Error: 图片加载失败！" << std::endl;
        return -1;
    }

    // 2. 定义矩形参数（在图片中心绘制）
    int rect_width = 100;  // 矩形宽度
    int rect_height = 50;  // 矩形高度
    cv::Point top_left(img.cols / 2 - rect_width / 2, img.rows / 2 - rect_height / 2); // 左上角坐标
    cv::Point bottom_right(top_left.x + rect_width, top_left.y + rect_height);         // 右下角坐标

    // 3. 绘制红色矩形（BGR格式：(0, 0, 255)）
    cv::rectangle(img, top_left, bottom_right, cv::Scalar(0, 0, 255), 2); // 线宽=2

    // 4. 显示结果
    cv::imshow("Image with Red Rectangle", img);
    cv::waitKey(0);

    // 5. 保存结果（可选）
    //cv::imwrite("output_with_rectangle.jpg", img);

    return 0;
}

//
//void UndistortImg(const double k, bool is_save, const string& path)
//{
//    cv::Mat img = cv::imread(R"(E:\code\media\grid.jpg)"); // 替换为你的图片路径
//    //新的相机内参矩阵
//    Mat new_intrinsic_mat;
//    img.copyTo(new_intrinsic_mat);
//    //调节视场大小,乘的系数越小视场越大
//    new_intrinsic_mat.at<double>(0, 0) *= k;
//    new_intrinsic_mat.at<double>(1, 1) *= k;
//
//    cout << endl << "开始校正图像" << endl;
//    for (int i = 0; i < img.size(); i++)
//    {
//        //调节校正图中心，建议置于校正图中心
//        new_intrinsic_mat.at<double>(0, 2) = 0.5 * img[i].cols;
//        new_intrinsic_mat.at<double>(1, 2) = 0.5 * img[i].rows;
//
//        Mat undistort_img;
//        fisheye::initUndistortRectifyMap(img[i], undistort_img, K, distortion_coeffs, new_intrinsic_mat);
//        //保存图像
//        if (is_save)
//        {
//            string filename = path + "/out" + format("%d", i + 1) + ".jpg";
//            imwrite(filename, undistort_img);
//        }
//    }
//    if (is_save)
//        cout << endl << "校正结果已保存至:" << path << endl;
//    cout << endl << "校正结束" << endl;
//}
