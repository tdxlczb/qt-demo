#include "media_frame_quality.h"
#include "media_utils.h"
#include "media_display.h"
#include "opencv2/opencv.hpp"
#include "log.h"

FrameQuality::FrameQuality()
{
}

FrameQuality::~FrameQuality()
{
}

bool FrameQuality::IsGrayFrame(const VideoFrame& frame)
{
    if (frame.spec.width <= 0 || frame.spec.height <= 0)
        return false;

    int iWidth = frame.spec.width;
    int iHeight = frame.spec.height;
    int gcd = GetGCD(iWidth, iHeight);
    int times = 0;
    if (gcd > 0) {
        int minW = iWidth / gcd;
        int minH = iHeight / gcd;
        iWidth = minW;
        iHeight = minH;
        //获取一个宽高都比100大的，比例一致的宽高，宽高比例要是不一致，重采样出来的图片是混乱的
        while (iWidth <= 100 && iHeight <= 100)
        {
            times += 2;
            iWidth = minW * times;
            iHeight = minH * times;
        }
    }
    VideoSpec spec;
    spec.width = iWidth;
    spec.height = iHeight;
    spec.format = 8;//AV_PIX_FMT_GRAY8

    if (!m_rgbConverter) {
        m_rgbConverter = std::make_unique<VideoConverter>(spec,"GRAY_CV");
    }

    VideoFrame outFrame;
    auto err = m_rgbConverter->Input(frame, outFrame);
    if (err.code != PlayErrorCode::kNoError)
        return false;

    try {//尝试捕捉opencv的错误

        // 将帧转换为灰度图像
        cv::Mat grayFrame = cv::Mat(outFrame.spec.height, outFrame.spec.width, CV_8UC1, outFrame.data);
        //检查灰度可以压缩图像大小，以加速检查
        //cv::resize(matRgb,grayFrame,cv::Size(100,100));
        //cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);

        //static int frameIndex = 0;
        //// 保存图片
        //char filename[100];
        //sprintf(filename, "E:\\code\\media\\temp\\%d.jpg", frameIndex);
        //cv::imwrite(filename, grayFrame);
        //frameIndex++;
        // 计算灰度图像的方差
        cv::Scalar mean, stddev;
        cv::meanStdDev(grayFrame, mean, stddev);
        double dGrayScaleDegree = stddev[0] * stddev[0];
        if (dGrayScaleDegree < 100.0) {
            // cv::imshow("gray",matRgb);
            // cv::waitKey(1);
            return true;
        }
    }
    catch (const std::bad_alloc& e) {
        // 捕获内存分配失败的异常
        LOG_ERROR << "memory allocation failed: " << e.what();
    }
    catch (const cv::Exception& e) {
        // 捕获 opencv 相关的异常
        LOG_ERROR << "opencv error: " << e.what();
    }
    catch (...) {
        // 捕获其他类型的异常
        LOG_ERROR << "unexpected error occurred.";
    }
    //cv::Mat mat = cv::Mat(frame->height, frame->width, CV_8UC3, frameRGB->data[0], frameRGB->linesize[0]);
    //bool isTooGray = false;
    //{
    //    // 将帧转换为灰度图像
    //    cv::Mat grayFrame;
    //    //检查灰度可以压缩图像大小，以加速检查
    //    cv::resize(mat, grayFrame, cv::Size(100, 100));
    //    cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);
    //    // 计算灰度图像的方差
    //    cv::Scalar mean, stddev;
    //    cv::meanStdDev(grayFrame, mean, stddev);
    //    double dGrayScaleDegree = stddev[0] * stddev[0];
    //    if (dGrayScaleDegree < 100.0) {
    //        isTooGray = true;
    //    }
    //}
    return false;
}