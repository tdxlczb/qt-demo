#ifndef FISHEYE_CORRECTION_H
#define FISHEYE_CORRECTION_H

#include <opencv2/opencv.hpp>

/*
* 剪切的上下左右量
* 0~width的数据保留left~(width-right)
* 0~height的数据保留top~(height-bottom)
*/
struct CropRect
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

/*
* 圆形展开类
*/
class UnwrapCircular
{
public:
    UnwrapCircular();
    ~UnwrapCircular();

    //设置和获取需要剪裁的数据大小
    void SetCropRect(CropRect rect);
    CropRect GetCropRect();
    ///
    /// \brief 获取展开后的图片数据
    /// \param src    图片数据，以图片中心为圆心坐标
    /// \param radius 半径
    /// \param isCrop 是否剪裁数据
    /// \return 返回展开后的图片数据
    ///
    cv::Mat GetUnwrapImage(const cv::Mat& src, int radius, bool isCrop = false);
    ///
    /// \brief 获取展开前的点坐标
    /// \param point 输入坐标
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
    CropRect m_cropRect;
    cv::Mat m_mapX;//映射矩阵
    cv::Mat m_mapY;//映射矩阵
};

/*
* 圆形拉伸类
*/
class StretchCircular
{
public:
    StretchCircular();
    ~StretchCircular();

    //设置和获取需要剪裁的数据大小
    void SetCropRect(CropRect rect);
    CropRect GetCropRect();

    ///
    /// \brief 获取拉伸后的图片数据
    /// \param src    图片数据，以图片中心为圆心坐标
    /// \param radius 半径
    /// \param isCrop 是否剪裁数据
    /// \return 返回展开后的图片数据
    ///
    cv::Mat GetStretchImage(const cv::Mat& src, int radius, bool isCrop = false);

private:
    ///
    /// \brief 创建圆形拉升的映射矩阵
    /// \param center 圆心坐标
    /// \param radius 半径
    /// \param stretchMode 拉升方式：0沿x轴拉伸，1沿y轴拉伸，2沿矩形边拉伸，3沿圆形边拉伸
    /// \return 
    ///
    void CreateMappingMatrix(const cv::Point& center, int radius, int stretchMode);
private:
    cv::Point m_center;
    int m_radius = 0;
    CropRect m_cropRect;
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


#endif // FISHEYE_CORRECTION_H
