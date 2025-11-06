#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

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
class VideoManager;
}

class PlayerWidget;
class VideoManager : public QWidget
{
    Q_OBJECT

public:
    explicit VideoManager(QWidget* parent = nullptr);
    ~VideoManager();

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onPlayAllClicked();
    void onStopAllClicked();
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

private:
    Ui::VideoManager* ui;
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
    QVector<QVector<PlayerWidget*>> videoCells;  // 3x3 视频单元格
    PlayerWidget* m_pSelectWidget = nullptr;

    // 右侧下层 - 控制面板
    //QWidget* controlPanel;
    //QPushButton* playButton;
    //QPushButton* stopButton;
    //QPushButton* playAllButton;
    //QPushButton* stopAllButton;
    QComboBox* comboBox;

    // 主布局
    QHBoxLayout* mainLayout;
};

#endif // VIDEO_MANAGER_H
