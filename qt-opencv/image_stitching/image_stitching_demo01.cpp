#include "image_stitching_demo01.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include <iostream>  

using namespace cv;
using namespace std;


#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int ImageStitchingDemo01() {
    // 1. 读取图像序列
    vector<Mat> images;
    images.push_back(imread(R"(E:\code\media\1.jpg)"));
    images.push_back(imread(R"(E:\code\media\2.jpg)"));
    images.push_back(imread(R"(E:\code\media\3.jpg)"));
    images.push_back(imread(R"(E:\code\media\4.jpg)"));

    // 检查图像是否有效
    for (size_t i = 0; i < images.size(); ++i) {
        if (images[i].empty()) {
            cout << "错误: 无法读取图像 " << i + 1 << endl;
            return -1;
        }
    }

    // 2. 创建全景拼接器
    Ptr<Stitcher> stitcher = Stitcher::create(Stitcher::PANORAMA);

    // 3. 设置拼接参数（可选）
    stitcher->setRegistrationResol(0.6);  // 图像缩放比例，加速特征匹配
    stitcher->setCompositingResol(1);     // 合成分辨率
    stitcher->setPanoConfidenceThresh(1); // 置信度阈值

    // 4. 执行拼接
    Mat panorama;
    Stitcher::Status status = stitcher->stitch(images, panorama);

    // 5. 处理结果
    if (status == Stitcher::OK) {
        imwrite("panorama_result.jpg", panorama);
        cout << "全景图拼接成功！尺寸: " << panorama.size() << endl;

        // 显示结果
        namedWindow("全景图", WINDOW_NORMAL);
        imshow("全景图", panorama);
        waitKey(0);

    }
    else {
        // 错误处理
        switch (status) {
        case Stitcher::ERR_NEED_MORE_IMGS:
            cout << "错误: 需要更多图像" << endl;
            break;
        case Stitcher::ERR_HOMOGRAPHY_EST_FAIL:
            cout << "错误: 单应性矩阵估计失败" << endl;
            break;
        case Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL:
            cout << "错误: 相机参数调整失败" << endl;
            break;
        default:
            cout << "未知错误: " << int(status) << endl;
        }
    }

    return 0;
}

int ImageStitchingDemo02()
{
    vector<Mat> images;
    images.push_back(imread(R"(E:\code\demo\fisheye_demo\IMG_20250903_143045.jpg)"));
    images.push_back(imread(R"(E:\code\demo\fisheye_demo\IMG_20250903_143048.jpg)"));
    cv::Mat img1 = images[0];
    cv::Mat img2 = images[1];
    // 1. 转换为灰度图
    Mat gray1, gray2;
    cvtColor(img1, gray1, COLOR_BGR2GRAY);
    cvtColor(img2, gray2, COLOR_BGR2GRAY);

    // 2. 特征检测 - 使用SIFT
    Ptr<SIFT> detector = SIFT::create();
    vector<KeyPoint> kp1, kp2;
    Mat des1, des2;

    detector->detectAndCompute(gray1, noArray(), kp1, des1);
    detector->detectAndCompute(gray2, noArray(), kp2, des2);

    // 3. 特征匹配
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector<vector<DMatch>> knn_matches;
    matcher->knnMatch(des1, des2, knn_matches, 2);

    // 4. Lowe's ratio test 筛选好的匹配
    vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++) {
        if (knn_matches[i][0].distance < 0.7f * knn_matches[i][1].distance) {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    // 5. 计算单应性矩阵
    vector<Point2f> pts1, pts2;
    for (const auto& m : good_matches) {
        pts1.push_back(kp1[m.queryIdx].pt);
        pts2.push_back(kp2[m.trainIdx].pt);
    }

    Mat H = findHomography(pts2, pts1, RANSAC);

    // 6. 透视变换和拼接
    Mat result;
    warpPerspective(img2, result, H, Size(img1.cols * 2, img1.rows * 1.5));

    // 7. 图像融合
    Mat roi(result, Rect(0, 0, img1.cols, img1.rows));
    img1.copyTo(roi);

    cv::imshow("show", result);
    waitKey();
    return 0;
}