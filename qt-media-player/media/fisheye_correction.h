#ifndef FISHEYE_CORRECTION_H
#define FISHEYE_CORRECTION_H

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

/*
* 圆形展开类
*/
class UnwrapCircular
{
public:
	UnwrapCircular();
	~UnwrapCircular();
    ///
    /// \brief 获取展开后的图片数据
    /// \param src    图片数据，以图片中心为圆心坐标
    /// \param radius 半径
    /// \return 返回展开后的图片数据
    ///
    cv::Mat GetUnwrapImage(const cv::Mat& src, int radius);
    ///
    /// \brief 获取展开前的点坐标
    /// \param point  展开后的点坐标
    /// \return 返回展开前的点坐标
    ///
    cv::Point2f GetOriginPoint(const cv::Point2f& point);
private:
    ///
    /// \brief 创建圆形展开的映射矩阵
    /// \param center 圆心坐标
    /// \param radius 半径
    /// \return 
    ///
    void CreateMappingMatrix(const cv::Point& center, int radius);
private:
    cv::Point m_center;
    int m_radius = 0;
    cv::Mat m_mapX;//映射矩阵
    cv::Mat m_mapY;//映射矩阵
};



class FishEyeCorrection
{
public:
    FishEyeCorrection();
    ~FishEyeCorrection();

    UnwrapCircular* GetUnwrapCircular();
    ///
    /// \brief 截取图片旋转矩形区域的数据
    /// \param src   源图片数据
    /// \param rotatedRect 旋转矩形
    /// \return 返回截取的数据
    ///
    cv::Mat ExtractRotatedRegionROI(const cv::Mat& src, const cv::RotatedRect& rotatedRect);
    ///
    /// \brief 判断点是否在旋转矩形区域内
    /// \param point   点坐标
    /// \param rotatedRect 旋转矩形
    /// \return 返回结果
    ///
    bool IsPointInRotatedRectTransform(const cv::Point2f& point, const cv::RotatedRect& rotatedRect);

private:


private:
    UnwrapCircular* pUnwrapCircular = nullptr;
};

cv::Mat UnwrapCircularImage(const cv::Mat& circularImage, int radius);
cv::Mat UnwrapCircularImageOptimized(const cv::Mat& circularImage, int radius);




#endif // FISHEYE_CORRECTION_H
