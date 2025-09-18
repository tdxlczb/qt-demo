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
    Ceiling,  // 顶装方式        (法线向下)
    Wall,     // 壁装方式        (法线水平)
    Floor,    // 地面安装        (法线向上)
};

//Normal是原图
//Panoramic是全景展开图，即圆形展开
//FishEye是鱼眼矫正图
enum class FECShowType
{
    Normal,
    NormalWith2PTZ,      //原图+2PTZ
    NormalWith3PTZ,      //原图+3PTZ
    NormalWith4PTZ,      //原图+4PTZ
    NormalWith8PTZ,      //原图+8PTZ
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

struct FishEyeChildInfo
{
    int row = 0;
    int column = 0;
    int rowSpan = 0;
    int columnSpan = 0;
    bool isMainChild = false;
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
    void CreatGridLayoutChild(const QList<FishEyeChildInfo>& list);
    void UpdateChildNormalContent(const cv::Mat& content);
    void UpdateChildPanoramicContent(const cv::Mat& content);
    void UpdateChildFishEyeContent(const cv::Mat& content);
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
    FECSetupType m_eSetupType = FECSetupType::Wall;
    FECShowType m_eShowType = FECShowType::Normal;
    QList<FishEyeChildWidget*> m_listWidgets;
    FishEyeChildWidget* m_pMainChildWidget = nullptr;
    QGridLayout* m_pGridLayout = nullptr;
};


#endif // FISHEYE_WIDGET_H
