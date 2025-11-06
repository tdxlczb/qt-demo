#include "video_manager.h"
#include "ui_video_manager.h"
#include "media_player/player_widget.h"

VideoManager::VideoManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoManager)
{
    ui->setupUi(this);
    setupUI();
    setWindowTitle("VideoManager");
    setMinimumSize(800, 600);
    resize(1200, 800);
}

VideoManager::~VideoManager()
{
    delete ui;
}


void VideoManager::setupUI()
{
    // 主布局 - 左右分层
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 设置左侧面板
    setupLeftPanel();

    // 设置右侧面板
    setupRightPanel();
}

void VideoManager::setupLeftPanel()
{
    // 左侧面板 - 固定宽度100，高度自适应
    leftPanel = new QWidget;
    leftPanel->setFixedWidth(100);
    leftPanel->setStyleSheet(
        "background-color: #2c3e50;"
        "border-right: 1px solid #34495e;"
    );

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 文件列表
    fileListWidget = new QListWidget;
    fileListWidget->setStyleSheet(
        "QListWidget {"
        "    background-color: #34495e;"
        "    border: none;"
        "    color: #ecf0f1;"
        "    font-size: 10px;"
        "}"
        "QListWidget::item {"
        "    border-bottom: 1px solid #2c3e50;"
        "    padding: 5px;"
        "    height: 30px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #2980b9;"
        "}"
    );

    // 添加示例文件
    QStringList sampleFiles;
    sampleFiles << "视频1.mp4" << "视频2.avi" << "视频3.mkv"
        << "视频4.mp4" << "视频5.avi" << "视频6.mkv"
        << "视频7.mp4" << "视频8.avi" << "视频9.mkv";

    fileListWidget->addItems(sampleFiles);

    leftLayout->addWidget(fileListWidget);

    // 连接信号槽
    connect(fileListWidget, &QListWidget::itemClicked, this, &VideoManager::onFileSelected);

    // 添加到主布局
    mainLayout->addWidget(leftPanel);
}

void VideoManager::setupRightPanel()
{
    // 右侧面板 - 自适应大小
    rightPanel = new QWidget;
    rightPanel->setStyleSheet("background-color: #ecf0f1;");

    // 右侧垂直布局 - 上下分层
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // 设置视频网格（上层）
    setupVideoGrid();

    // 设置控制面板（下层）
    setupControlPanel();

    // 添加到主布局，拉伸因子为1使其自适应
    mainLayout->addWidget(rightPanel, 1);
}

void VideoManager::setupVideoGrid()
{
    // 视频网格区域 - 自适应大小
    videoGridWidget = new QWidget;
    videoGridWidget->setStyleSheet("background-color: #000000;");

    // 创建 3x3 网格布局
    videoGridLayout = new QGridLayout(videoGridWidget);
    videoGridLayout->setSpacing(2);
    videoGridLayout->setContentsMargins(2, 2, 2, 2);

    // 初始化视频单元格容器
    videoCells.resize(3);
    for (int i = 0; i < 3; ++i) {
        videoCells[i].resize(3);
    }

    // 创建 3x3 视频单元格
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            createVideoCell(row, col);
        }
    }

    // 添加到右侧布局，拉伸因子为1使其自适应
    rightLayout->addWidget(videoGridWidget, 1);
}

void VideoManager::createVideoCell(int row, int col)
{
    //// 创建单个视频单元格
    //QWidget* cell = new QWidget;
    //cell->setStyleSheet(
    //    "background-color: #1a1a1a;"
    //    "border: 1px solid #444;"
    //    "border-radius: 3px;"
    //);

    //QVBoxLayout* cellLayout = new QVBoxLayout(cell);
    //cellLayout->setContentsMargins(5, 5, 5, 5);
    //cellLayout->setAlignment(Qt::AlignCenter);

    //// 视频编号标签
    //QLabel* numberLabel = new QLabel(QString("视频 %1").arg(row * 3 + col + 1));
    //numberLabel->setStyleSheet(
    //    "color: #ffffff;"
    //    "font-size: 14px;"
    //    "font-weight: bold;"
    //    "background: transparent;"
    //);
    //numberLabel->setAlignment(Qt::AlignCenter);

    //// 状态标签
    //QLabel* statusLabel = new QLabel("未播放");
    //statusLabel->setStyleSheet(
    //    "color: #95a5a6;"
    //    "font-size: 10px;"
    //    "background: transparent;"
    //);
    //statusLabel->setAlignment(Qt::AlignCenter);

    //cellLayout->addWidget(numberLabel);
    //cellLayout->addWidget(statusLabel);

    // 存储到容器中
    //videoCells[row][col] = cell;
    int index = row * 3 + col + 1;
    PlayerWidget* playWidget = new PlayerWidget(nullptr, index);
    connect(playWidget, &PlayerWidget::sig_Selected, this, &VideoManager::on_Selected);
    videoCells[row][col] = playWidget;
    // 添加到网格布局
    videoGridLayout->addWidget(playWidget, row, col);
}

void VideoManager::setupControlPanel()
{
    // 控制面板 - 固定高度50
    controlPanel = new QWidget;
    controlPanel->setFixedHeight(50);
    controlPanel->setStyleSheet(
        "background-color: #34495e;"
        "border-top: 1px solid #2c3e50;"
    );

    QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(20, 5, 20, 5);
    controlLayout->setSpacing(15);

    // 播放所有按钮
    playAllButton = new QPushButton("播放全部");
    playAllButton->setFixedSize(100, 30);
    playAllButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2ecc71;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #229954;"
        "}"
    );

    // 停止所有按钮
    stopAllButton = new QPushButton("停止全部");
    stopAllButton->setFixedSize(100, 30);
    stopAllButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ec7063;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0392b;"
        "}"
    );

    // 添加到布局
    controlLayout->addStretch();
    controlLayout->addWidget(playAllButton);
    controlLayout->addWidget(stopAllButton);
    controlLayout->addStretch();

    // 连接信号槽
    connect(playAllButton, &QPushButton::clicked, this, &VideoManager::onPlayAllClicked);
    connect(stopAllButton, &QPushButton::clicked, this, &VideoManager::onStopAllClicked);

    // 添加到右侧布局
    rightLayout->addWidget(controlPanel);
}

void VideoManager::onPlayAllClicked()
{
    // 播放全部视频的逻辑
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            //QWidget* cell = videoCells[row][col];
            //QLabel* statusLabel = cell->findChild<QLabel*>("", Qt::FindDirectChildrenOnly);
            //if (statusLabel && statusLabel->text() != "状态") {
            //    statusLabel->setText("播放中...");
            //    statusLabel->setStyleSheet("color: #2ecc71; font-size: 10px;");
            //}

            //// 设置单元格背景色表示播放状态
            //cell->setStyleSheet(
            //    "background-color: #2c3e50;"
            //    "border: 2px solid #3498db;"
            //    "border-radius: 3px;"
            //);
            //QString playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000002_5?token=G9dSZrnumeb1TDSf";//2560
            QString playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000006_5?token=WSGLtsoIcY7bf25L";//2880
            videoCells[row][col]->StartPlay(playUrl);
        }
    }

    // 这里添加实际的播放逻辑
    // for (auto& player : videoPlayers) {
    //     player->play();
    // }
}

void VideoManager::onStopAllClicked()
{
    // 停止全部视频的逻辑
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            //QWidget* cell = videoCells[row][col];
            //QLabel* statusLabel = cell->findChild<QLabel*>("", Qt::FindDirectChildrenOnly);
            //if (statusLabel && statusLabel->text() != "状态") {
            //    statusLabel->setText("已停止");
            //    statusLabel->setStyleSheet("color: #e74c3c; font-size: 10px;");
            //}

            //// 恢复单元格默认样式
            //cell->setStyleSheet(
            //    "background-color: #1a1a1a;"
            //    "border: 1px solid #444;"
            //    "border-radius: 3px;"
            //);
            videoCells[row][col]->StopPlay();
        }
    }

    // 这里添加实际的停止逻辑
    // for (auto& player : videoPlayers) {
    //     player->stop();
    // }
}

void VideoManager::onFileSelected(QListWidgetItem* item)
{
    if (!item) return;

    QString fileName = item->text();
    // 这里可以添加文件选择后的处理逻辑
    // 比如在某个视频窗口中播放选中的文件

    // 示例：在第一个视频窗口中显示选中的文件名
    if (videoCells.size() > 0 && videoCells[0].size() > 0) {
        QWidget* firstCell = videoCells[0][0];
        QLabel* numberLabel = firstCell->findChild<QLabel*>("", Qt::FindDirectChildrenOnly);
        if (numberLabel) {
            numberLabel->setText(fileName);
        }
    }
}

void VideoManager::on_Selected(PlayerWidget* pWidget)
{
    if (m_pSelectWidget) {
        m_pSelectWidget->setStyleSheet("background-color: #F0F0F0; border: none;");
    }
    pWidget->setStyleSheet("border: 2px solid red;");
    m_pSelectWidget = pWidget;
}
