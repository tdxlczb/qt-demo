#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <opencv2/opencv.hpp>
#include <cmath>

/*
* 圆环扇形
*/
class CircularSector {
public:
    CircularSector(){};
    // 支持普通扇形和圆环扇形
    CircularSector(cv::Point center, float startAngle, float endAngle, int outerRadius, int innerRadius = 0);
    ~CircularSector();

    // 绘制扇形/圆环扇形边框
    void DrawBorder(cv::Mat& image, const cv::Scalar& color, int thickness = 2) const;

    // 绘制填充扇形/圆环扇形
    void DrawFilled(cv::Mat& image, const cv::Scalar& fillColor, const cv::Scalar& borderColor = cv::Scalar(0, 0, 0), int borderThickness = 1) const;

    // 判断点是否在扇形/圆环扇形区域内
    bool IsPointInRegion(const cv::Point& point) const;

    // 计算扇形角度
    float GetSectorAngle() const;

    // 计算扇形面积
    float CalculateArea() const;

private:
    cv::Point m_center;
    int m_outerRadius = 0;   // 外半径
    int m_innerRadius = 0;   // 内半径（为0时是普通扇形）
    float m_startAngle = 0.0f;    // 起始角度（度）
    float m_endAngle = 0.0f;      // 结束角度（度）

};


/*
* 旋转矩形, 正对环绕中心点
*/
class RotatedRectangle {
public:
    RotatedRectangle(){};
    ///
    /// \brief 构造函数
    /// \param center 环绕的中心点
    /// \param size   矩形大小
    /// \param radius 环绕半径，如果环绕半径为0，则环绕角度为矩形的旋转角度
    /// \param angle  环绕角度(90，180)
    /// \return 返回展开后的图片数据
    ///
    RotatedRectangle(cv::Point center, cv::Size size, int radius, float angle);
    ~RotatedRectangle();

    // 绘制边框
    void DrawBorder(cv::Mat& image, const cv::Scalar& color, int thickness = 2) const;

    // 绘制填充
    void DrawFilled(cv::Mat& image, const cv::Scalar& fillColor, const cv::Scalar& borderColor = cv::Scalar(0, 0, 0), int borderThickness = 1) const;

    // 判断点是否在区域内
    bool IsPointInRegion(const cv::Point& point) const;
private:
    void CreateRotatedRect(cv::Size size);
private:
    cv::Point m_center;//环绕的中心点
    int m_radius = 0;//环绕的半径
    float m_angle = 0.0f;//环绕的角度，和RotatedRect中的角度不一致，应该是垂直关系
    cv::RotatedRect m_rotatedRect;
};


/*
* 自定义多边形
*/
class CustomPolygon {
public:
    CustomPolygon() {};
    ///
    /// \brief 构造函数
    /// \param center 环绕的中心点
    /// \param size   矩形大小
    /// \param radius 环绕半径，如果环绕半径为0，则环绕角度为矩形的旋转角度
    /// \param angle  环绕角度(90，180)
    /// \return 返回展开后的图片数据
    ///
    CustomPolygon(cv::Point center, cv::Size size, int radius, float angle);
    ~CustomPolygon();

    // 绘制边框
    void DrawBorder(cv::Mat& image, const cv::Scalar& color, int thickness = 2) const;

    // 绘制填充
    void DrawFilled(cv::Mat& image, const cv::Scalar& fillColor, const cv::Scalar& borderColor = cv::Scalar(0, 0, 0), int borderThickness = 1) const;

    // 判断点是否在区域内
    bool IsPointInRegion(const cv::Point& point) const;
private:
    void CreateRotatedRect(cv::Size size);
private:
    cv::Point m_center;//环绕的中心点
    int m_radius = 0;//环绕的半径
    float m_angle = 0.0f;//环绕的角度，和RotatedRect中的角度不一致，应该是垂直关系
    cv::RotatedRect m_rotatedRect;
};

#endif // GRAPHICS_H
