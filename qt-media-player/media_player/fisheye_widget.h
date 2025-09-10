#ifndef FISHEYE_WIDGET_H
#define FISHEYE_WIDGET_H

#include <QGridLayout>
#include <QWidget>
#include <QMutex>
#include <QImage>
#include <QList>
#include <opencv2/core.hpp>

enum class FECSetupType
{
    None,
    Top,    //顶装(法线向下）
    Side,   //壁装（法线水平）
    Bottom  //底装(法线向上）
};

enum class FECShowType
{
    Normal,
    Panoramic,           //全景
    PanoramicWith3PTZ,   //全景+3PTZ
    PanoramicWith8PTZ,   //全景+8PTZ
    Panoramic180,        //全景180
    Panoramic360,        //全景360
    Panoramic360With1PTZ,//全景360+1PTZ
    Panoramic360With3PTZ,//全景360+3PTZ
    Panoramic360With6PTZ,//全景360+6PTZ
    Panoramic360With8PTZ,//全景360+8PTZ
    FishEyeWith2PTZ,     //鱼眼+2PTZ
    FishEyeWith3PTZ,     //鱼眼+3PTZ
    FishEyeWith4PTZ,     //鱼眼+4PTZ
    FishEyeWith8PTZ,     //鱼眼+8PTZ
    HalfSphere,          //半球
    HalfSphereAR,        //AR半球
    cylinder             //圆柱
};

class FishEyeCorrection;
class FishEyeChildWidget;
class FishEyeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FishEyeWidget(QWidget* parent = nullptr);
    ~FishEyeWidget();

    void SetFishEyeType(FECSetupType eSetupType, FECShowType eShowType);
    void UpdateContent(const cv::Mat& content);

signals:
    void sig_Update();

public slots:
    void on_Update();
    void on_ChildMousePress(const QPoint& point);
    void on_ChildMouseMove(const QPoint& point);
    void on_ChildMouseRelease(const QPoint& point);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
private:
    void UpdateContentSafe(const cv::Mat& content);
private:
    cv::Mat m_content;
    QMutex m_contentMutex;
    QPoint m_pointPress = { 0,0 };//鼠标按钮的坐标
    QPoint m_pointMove = { 0,0 };//鼠标移动的坐标
    QPoint m_pointRelease = { 0,0 };//鼠标弹起的坐标
    int m_selectedIndex = -1;
    int m_lastScrollOffset = 0;//图片拖拽滚动的值
    int m_curScrollOffset = 0;//图片拖拽滚动的值

    FishEyeCorrection* m_pFishEyeCorrection = nullptr;
    FECSetupType m_eSetupType = FECSetupType::Top;
    FECShowType m_eShowType = FECShowType::Normal;
    QList<FishEyeChildWidget*> m_listWidgets;
    FishEyeChildWidget* m_pMainChildWidget = nullptr;
    QGridLayout* m_pGridLayout = nullptr;
};


#endif // FISHEYE_WIDGET_H
