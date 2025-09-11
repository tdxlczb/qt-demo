#include "fisheye_correction_demo01.h"

#include <iostream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
using namespace std;
using namespace cv;


// 将圆形图像展开成矩形
Mat unwrapCircularImage(const Mat& circularImage, int radius = -1) {
    // 确定圆的半径（如果未指定，使用图像的最小边长的一半）
    int R = (radius > 0) ? radius : min(circularImage.cols, circularImage.rows) / 2;

    // 圆心坐标
    Point center(circularImage.cols / 2, circularImage.rows / 2);

    // 创建展开后的矩形图像：宽度为圆周长 2πR，高度为半径 R
    int rectWidth = round(2 * CV_PI * R);
    int rectHeight = R;
    Mat unwrappedImage(rectHeight, rectWidth, circularImage.type(), Scalar(0));

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
                unwrappedImage.at<Vec3b>(y, x) = circularImage.at<Vec3b>(srcY, srcX);
            }
        }
    }

    return unwrappedImage;
}

//优化使用remap创建映射矩阵实现，不使用使用映射矩阵，每次计算耗时较高，不能用于处理视频图像
Mat unwrapCircularImageOptimized(const Mat& circularImage, int radius = -1) {
    int R = (radius > 0) ? radius : min(circularImage.cols, circularImage.rows) / 2;
    Point center(circularImage.cols / 2, circularImage.rows / 2);

    int rectWidth = round(2 * CV_PI * R);
    int rectHeight = R;

    // 创建映射矩阵
    Mat map_x(rectHeight, rectWidth, CV_32F);
    Mat map_y(rectHeight, rectWidth, CV_32F);

    for (int y = 0; y < rectHeight; y++) {
        for (int x = 0; x < rectWidth; x++) {
            double theta = (2 * CV_PI * x) / rectWidth;
            double radius = y;

            map_x.at<float>(y, x) = center.x + radius * cos(theta);
            map_y.at<float>(y, x) = center.y + radius * sin(theta);
        }
    }

    Mat unwrappedImage;
    remap(circularImage, unwrappedImage, map_x, map_y, INTER_LINEAR, BORDER_CONSTANT);

    return unwrappedImage;
}

// 反向操作：将矩形图像重新卷成圆形
Mat rewrapToCircularImage(const Mat& unwrappedImage, Size circularSize) {
    Mat circularImage(circularSize, unwrappedImage.type(), Scalar(0));

    int R = unwrappedImage.rows;  // 矩形高度就是半径
    Point center(circularSize.width / 2, circularSize.height / 2);
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
                    circularImage.at<Vec3b>(y, x) = unwrappedImage.at<Vec3b>(rectY, rectX);
                }
            }
        }
    }

    return circularImage;
}

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
    cv::rectangle(circularImage, top_left, bottom_right, cv::Scalar(0, 0, 255), 2); // 线宽=2

    int radius = min(circularImage.cols, circularImage.rows) / 2;
    cv::resize(circularImage, circularImage, cv::Size(radius * 2, radius * 2));

    // 展开圆形图像
    Mat unwrapped = unwrapCircularImageOptimized(circularImage, radius);
    //unwrapped = unwrapped(Rect(0, 60, unwrapped.cols, unwrapped.rows - 60));//裁剪顶部畸变严重的部分
    // 显示结果
    imshow("原始圆形图像", circularImage);
    imshow("展开后的矩形图像", unwrapped);

    // 可选：重新卷回圆形（验证算法正确性）
    Mat rewrapped = rewrapToCircularImage(unwrapped, circularImage.size());
    imshow("重新卷回的圆形图像", rewrapped);

    waitKey(0);
    return 0;
}

int FisheyeDemo01() {
    cv::Mat frame = cv::imread(R"(E:\code\media\image\Snipaste_2025-09-10_20-18-10.jpg)");
    cv::Mat after;
    // 相机内参矩阵（根据自己的相机参数修改）
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 286.1809997558594, 0.0, 421.6383972167969,
        0.0, 286.3576965332031, 403.9013977050781,
        0.0, 0.0, 1.0);
    // 畸变系数 - 假设有4个: k1, k2, k3, k4，（根据自己的相机参数修改）
    cv::Mat distCoeffs = (cv::Mat_<double>(4, 1) << -0.008326118811964989, 0.04620290920138359, -0.04403631016612053, 0.00837636087089777);

    // 计算最优新相机内参矩阵
    cv::Size imageSize(800, 800);
    cv::Mat newCameraMatrix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);
    //注意，如果是处理视频，建议将该函数内部实现抽离，映射矩阵只需要生成一次即可，可以优化效率
    cv::fisheye::undistortImage(frame, after, cameraMatrix, distCoeffs, newCameraMatrix);

    cv::imshow("Original", frame);
    cv::imshow("Fisheye", after);
    cv::waitKey(0);
    return 0;
}

#include <opencv2/opencv.hpp>
#include <cmath>

int FisheyeDemo02() {
    cv::Mat src = cv::imread(R"(E:\code\media\image\grid.jpg)");
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

            if (x == 380 && y == 799) {
                int i = 0;
            }
            // 归一化并应用非线性变换（模拟鱼眼）
            float normalizedDist = distance / maxRadius;
            if (normalizedDist > 1.0f) normalizedDist = 1.0f;

            // 调整畸变强度（0.5 是控制参数）
            float fisheyeDist = std::pow(normalizedDist, 2.0f) * maxRadius;

            // 计算原始图像中的对应点
            if (distance > 0) {
                float angle = std::atan2(dy, dx);
                float degree = 360 * angle / CV_2PI;
                // 使用绝对值简化计算
                double cos_val = std::abs(std::cos(angle));
                double sin_val = std::abs(std::sin(angle));
                // 距离公式：r = a / max(|cosθ|, |sinθ|)
                float rectDistance = maxRadius / std::max(cos_val, sin_val);

                //float scale = distance / rectDistance;
                //float ps = std::pow(scale, 2.0f);
                //// 调整畸变强度（0.5 是控制参数）
                //float fisheyeDist = std::pow(scale, 0.5f) * maxRadius;
                ////float fisheyeDist = scale * maxRadius;

                float srcX = center.x + fisheyeDist * std::cos(angle);
                float srcY = center.y + fisheyeDist * std::sin(angle);

                // 双线性插值
                if (srcX >= 0 && srcX < src.cols - 1 && srcY >= 0 && srcY < src.rows - 1) {
                    dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(cv::Point(srcX, srcY));
                }
            }
            else {
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
#include <cmath>

using namespace cv;
using namespace std;

Mat applyFisheyeEffect(const Mat& input) {
    Mat output = Mat::zeros(input.size(), input.type());
    Mat map_x = Mat::zeros(input.size(), CV_32F);
    Mat map_y = Mat::zeros(input.size(), CV_32F);

    int width = input.cols;
    int height = input.rows;
    Point center(width / 2, height / 2);
    float radius = min(center.x, center.y);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算当前点到中心的距离
            float dx = x - center.x;
            float dy = y - center.y;
            float distance = sqrt(dx * dx + dy * dy);

            // 如果点在鱼眼半径内
            if (distance <= radius) {
                float r = distance / radius;
                float theta = atan2(dy, dx);

                // 非线性映射 - 增加鱼眼效果
                float new_r = std::pow(r, 0.5f);  // 二次方增强效果

                // 计算新位置
                float new_x = center.x + new_r * radius * cos(theta);
                float new_y = center.y + new_r * radius * sin(theta);

                map_x.at<float>(y, x) = new_x;
                map_y.at<float>(y, x) = new_y;
            }
            else {
                // 保持原样
                map_x.at<float>(y, x) = x;
                map_y.at<float>(y, x) = y;
            }
        }
    }

    remap(input, output, map_x, map_y, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));
    return output;
}

int FisheyeDemo03() {
    Mat image = imread(R"(E:\code\media\image\grid.jpg)");
    if (image.empty()) {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    Mat fisheyeImage = applyFisheyeEffect(image);

    imshow("Original Image", image);
    imshow("Fisheye Effect", fisheyeImage);
    waitKey(0);

    return 0;
}

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat createFisheyeEffect(const Mat& src) {
    Mat dst;
    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0, 0) = 1000;  // fx
    cameraMatrix.at<double>(1, 1) = 1000;  // fy
    cameraMatrix.at<double>(0, 2) = src.cols / 2;  // cx
    cameraMatrix.at<double>(1, 2) = src.rows / 2;  // cy

    Mat distCoeffs = Mat::zeros(4, 1, CV_64F);
    distCoeffs.at<double>(0) = 50.0;   // k1
    distCoeffs.at<double>(1) = 3.0;   // k2
    distCoeffs.at<double>(2) = 1.0;   // p1
    distCoeffs.at<double>(3) = 1.0;   // p2

    fisheye::undistortImage(src, dst, cameraMatrix, distCoeffs, cameraMatrix);
    return dst;
}

Mat applyFisheyeWithCalibration(const Mat& inputImage,
    const Mat& cameraMatrix,
    const Mat& distCoeffs) {
    Mat map1, map2;
    Size imageSize = inputImage.size();

    // 创建畸变映射
    fisheye::initUndistortRectifyMap(
        cameraMatrix, distCoeffs, Mat::eye(3, 3, CV_32F),
        cameraMatrix, imageSize, CV_32FC1, map1, map2
    );

    // 应用反向映射（从矫正到畸变）
    Mat distortedImage;
    remap(inputImage, distortedImage, map1, map2, INTER_LINEAR, BORDER_CONSTANT);

    return distortedImage;
}

int FisheyeDemo04() {
    //Mat image = imread(R"(E:\code\media\image\hediquanjingyuyan-22918431_3.jpg)");
    //if (image.empty()) {
    //    cout << "Could not open or find the image" << endl;
    //    return -1;
    //}

    //Mat fisheyeImage = createFisheyeEffect(image);

    //imshow("Original Image", image);
    //imshow("Fisheye Effect", fisheyeImage);
    //waitKey(0);


    // 读取输入图像
    Mat inputImage = imread(R"(E:\code\media\image\03751760ffcec8fdd2e547cee875ee02.jpg)");
    if (inputImage.empty()) {
        cerr << "无法读取输入图像！" << endl;
        return -1;
    }

    // 创建鱼眼相机参数（这些值需要根据实际情况调整）
    Mat cameraMatrix = (Mat_<double>(3, 3) <<
        500, 0, inputImage.cols / 2.0,
        0, 500, inputImage.rows / 2.0,
        0, 0, 1);

    // 鱼眼畸变系数 [k1, k2, k3, k4]
    Mat distCoeffs = (Mat_<double>(4, 1) << 0.3, 0.1, 0.05, 0.01);

    Mat fisheyeImage = applyFisheyeWithCalibration(inputImage, cameraMatrix, distCoeffs);

    imshow("原始图像", inputImage);
    imshow("鱼眼畸变效果", fisheyeImage);
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
