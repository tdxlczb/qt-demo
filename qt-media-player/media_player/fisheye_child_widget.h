#ifndef FISHEYE_CHILD_WIDGET_H
#define FISHEYE_CHILD_WIDGET_H

#include <QWidget>
#include <QMutex>
#include <opencv2/core.hpp>

class FishEyeCorrection;
class FishEyeWidget;
class FishEyeChildWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FishEyeChildWidget(int index, bool isMainChild, FishEyeWidget* parent);
    ~FishEyeChildWidget();

    bool IsMainChild();
    void SetSelected(bool isSelected);
    bool IsSelected();
    void SetDefaultRotatedRect(float rotateRadiusScale, float rotateDegree);//角度
    cv::RotatedRect GetRotatedRect();
    void UpdateOriginSize(int originWidth, int originHeight);
    void UpdateRotatedRect(int rotateRadius, float rotateAngle);//弧度
    void UpdateRotatedRect(int moveX, int moveY, bool isRelease);
    void UpdateContent(const cv::Mat& content);

signals:
    void sig_Update();
    void sig_ChildMousePress(const QPoint& point);
    void sig_ChildMouseMove(const QPoint& point);
    void sig_ChildMouseRelease(const QPoint& point);

public slots:
    void on_Update();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    //void keyPressEvent(QKeyEvent* event) override;
    //void keyReleaseEvent(QKeyEvent* event) override;
    //void focusInEvent(QFocusEvent* event) override;
    //void focusOutEvent(QFocusEvent* event) override;
    //void enterEvent(QEvent* event) override;
    //void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    //void moveEvent(QMoveEvent* event) override;
    //void resizeEvent(QResizeEvent* event) override;
private:
    void CheckRotateRadius(int& radius);
    int GetOriginRadius();
private:
    int m_index = 0;
    bool m_isMainChild = false;//是否为主子窗口
    cv::Mat m_content;
    QMutex m_contentMutex;
    QPoint m_pointPress = { 0,0 };//鼠标按钮的坐标
    QPoint m_pointMove = { 0,0 };//鼠标移动的坐标
    QPoint m_pointRelease = { 0,0 };//鼠标弹起的坐标

    bool m_isSelected = false;
    float m_defaultRotateRadiusScale = 0.0f;
    float m_defaultRotateAngle = 0.0f;//弧度
    float m_rotateRectScale = 1.0f;
    float m_scaleX = 1.0f;
    float m_scaleY = 1.0f;
    const float kMaxRadiusScale = 0.75f;
    const float kMinRadiusScale = 0.01f;
    int m_originWidth = 0;
    int m_originHeight = 0;
    int m_lastRotateRadius = 0;
    float m_lastRotateAngle = 0.0f;//弧度
    cv::RotatedRect m_rotatedRect;
};

#endif // FISHEYE_CHILD_WIDGET_H
