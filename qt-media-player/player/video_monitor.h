#ifndef VIDEO_MONITOR_H
#define VIDEO_MONITOR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QList>

namespace Ui {
class VideoMonitor;
}

class PlayerWidget;
class VideoMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit VideoMonitor(QWidget* parent = nullptr);
    ~VideoMonitor();

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onPlayAllClicked();
    void onStopAllClicked();
    void onSplitComboBoxChanged(int index);
    void onFileSelected(QListWidgetItem* item);
    //窗口被选中
    void on_Selected(PlayerWidget* pWidget);

private:
    QList<QString> readUrlList();
    void saveUrlList(const QList<QString>& urlList);
    void setupUI();
    void setupLeftPanel();
    void setupUrlList();
    void setupUrlListControlPanel();
    void setupRightPanel();
    void setupVideoGrid();
    void setupControlPanel();
    void createVideoCell(int row, int col);
    void updateVideoGrid();

private:
    Ui::VideoMonitor* ui;
    // 左侧文件列表
    QWidget* leftPanel;
    QVBoxLayout* leftLayout;
    QListWidget* fileListWidget;

    // 右侧主区域
    QWidget* rightPanel;
    QVBoxLayout* rightLayout;

    // 右侧上层 - 视频网格
    QWidget* videoGridWidget;
    QGridLayout* videoGridLayout;
    QVector<PlayerWidget*> videoCells;  //视频单元格
    PlayerWidget* m_pSelectWidget = nullptr;

    // 右侧下层 - 控制面板
    //QWidget* controlPanel;
    //QPushButton* playButton;
    //QPushButton* stopButton;
    //QPushButton* playAllButton;
    //QPushButton* stopAllButton;
    QComboBox* decodeComboBox;
    QComboBox* splitComboBox;

    // 主布局
    QHBoxLayout* mainLayout;


    int gridRows = 2;
    int gridCols = 2;
};

#endif // VIDEO_MONITOR_H
