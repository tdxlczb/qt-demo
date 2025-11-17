#include "media_player.h"
#include "ui_media_player.h"
#include <QTime>
#include <QStyle>
#include <QFileDialog>
#include <QResizeEvent>
#include <QDebug>
#include "player_widget.h"

MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaPlayer)
    , m_pPlayerWidget(new PlayerWidget(this))
{
    ui->setupUi(this);

    setupUI();
    setWindowTitle("MediaPlayer");
    setMinimumSize(800, 600); // 设置最小尺寸
    resize(800, 600);
}

MediaPlayer::~MediaPlayer()
{
    delete ui;
}

void MediaPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void MediaPlayer::setupUI()
{
    // 主布局 - 上中下四层
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0); // 无间距
    mainLayout->setContentsMargins(0, 0, 0, 0); // 无边距

    // 设置一层 - 播放窗口
    setupVideoArea();

    // 设置二层 - 进度条区域
    setupProgressArea();

    // 设置三层 - 信息编辑区域
    setupInfoArea();

    // 设置四层 - 控制按钮区域
    setupControlArea();
}

void MediaPlayer::setupVideoArea()
{
    // 一层 - 播放窗口（自适应）
    videoWidget = new QWidget;
    videoWidget->setStyleSheet(
        "background-color: #000000;"
        "border: none;"
    );
    videoWidget->setMinimumHeight(200); // 最小高度

    QVBoxLayout* videoLayout = new QVBoxLayout(videoWidget);
    videoLayout->setContentsMargins(0, 0, 0, 0);

    videoLabel = new QLabel("视频播放");
    videoLabel->setStyleSheet(
        "color: #ffffff;"
        "font-size: 18px;"
        "font-weight: bold;"
        "background: transparent;"
    );
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLayout->addWidget(videoLabel);
    videoLayout->addWidget(m_pPlayerWidget);
    m_pPlayerWidget->hide();
    // 添加到主布局，拉伸因子为1使其自适应
    mainLayout->addWidget(videoWidget, 1);
}

void MediaPlayer::setupProgressArea()
{
    // 二层 - 进度条区域（固定高度50）
    progressWidget = new QWidget;
    progressWidget->setFixedHeight(50);
    progressWidget->setStyleSheet(
        "background-color: #2c3e50;"
        "border-top: 1px solid #34495e;"
        "border-bottom: 1px solid #34495e;"
    );

    QHBoxLayout* progressLayout = new QHBoxLayout(progressWidget);
    progressLayout->setContentsMargins(15, 5, 15, 5);
    progressLayout->setSpacing(10);

    // 当前时间标签
    currentTimeLabel = new QLabel("00:00");
    currentTimeLabel->setStyleSheet(
        "color: #ecf0f1;"
        "font-size: 12px;"
        "min-width: 40px;"
        "background: transparent;"
    );
    currentTimeLabel->setAlignment(Qt::AlignCenter);

    // 进度条
    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);
    progressSlider->setValue(0);
    progressSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    border: 1px solid #34495e;"
        "    height: 6px;"
        "    background: #34495e;"
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #3498db;"
        "    border: 1px solid #2980b9;"
        "    width: 14px;"
        "    margin: -4px 0;"
        "    border-radius: 7px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #3498db;"
        "    border-radius: 3px;"
        "}"
    );

    // 总时间标签
    totalTimeLabel = new QLabel("05:00");
    totalTimeLabel->setStyleSheet(
        "color: #ecf0f1;"
        "font-size: 12px;"
        "min-width: 40px;"
        "background: transparent;"
    );
    totalTimeLabel->setAlignment(Qt::AlignCenter);

    // 添加到布局
    progressLayout->addWidget(currentTimeLabel);
    progressLayout->addWidget(progressSlider, 1); // 进度条拉伸
    progressLayout->addWidget(totalTimeLabel);

    // 连接信号槽
    connect(progressSlider, &QSlider::valueChanged, this, &MediaPlayer::onProgressChanged);

    mainLayout->addWidget(progressWidget);
}

void MediaPlayer::setupInfoArea()
{
    // 三层 - 信息编辑区域（固定高度50）
    infoWidget = new QWidget;
    infoWidget->setFixedHeight(50);
    infoWidget->setStyleSheet(
        "background-color: #34495e;"
        "border-top: 1px solid #2c3e50;"
        "border-bottom: 1px solid #2c3e50;"
    );

    QHBoxLayout* infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(10, 5, 10, 5);
    infoLayout->setSpacing(10);

    // 打开按钮
    openButton = new QPushButton("打开文件");
    openButton->setFixedSize(80, 30);
    openButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );

    // 可编辑的文本区域
    infoTextEdit = new QTextEdit;
    infoTextEdit->setPlaceholderText("在此输入文件路径或备注信息...");
    infoTextEdit->setMaximumHeight(30); // 限制高度
    infoTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏垂直滚动条
    infoTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 3px;"
        "    padding: 2px 5px;"
        "    font-size: 12px;"
        "    selection-background-color: #3498db;"
        "}"
        "QTextEdit:focus {"
        "    border: 1px solid #3498db;"
        "}"
    );

    // 添加到布局
    infoLayout->addWidget(openButton);
    infoLayout->addWidget(infoTextEdit, 1); // 文本编辑框拉伸

    // 连接信号槽
    connect(openButton, &QPushButton::clicked, this, &MediaPlayer::onOpenClicked);
    connect(infoTextEdit, &QTextEdit::textChanged, this, &MediaPlayer::onTextChanged);

    mainLayout->addWidget(infoWidget);
}

void MediaPlayer::setupControlArea()
{
    // 四层 - 控制按钮区域（固定高度50）
    controlWidget = new QWidget;
    controlWidget->setFixedHeight(50);
    controlWidget->setStyleSheet(
        "background-color: #2c3e50;"
        "border-top: 1px solid #34495e;"
    );

    QHBoxLayout* controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(20, 5, 20, 5);
    controlLayout->setSpacing(15);

    // 播放按钮
    playButton = new QPushButton;
    playButton->setFixedSize(40, 40);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    border: none;"
        "    border-radius: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2ecc71;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #229954;"
        "}"
    );

    // 暂停按钮
    pauseButton = new QPushButton;
    pauseButton->setFixedSize(40, 40);
    pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    pauseButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f39c12;"
        "    border: none;"
        "    border-radius: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1c40f;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e67e22;"
        "}"
    );

    // 停止按钮
    stopButton = new QPushButton;
    stopButton->setFixedSize(40, 40);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #e74c3c;"
        "    border: none;"
        "    border-radius: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ec7063;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0392b;"
        "}"
    );

    // 添加到布局
    controlLayout->addStretch(); // 左侧弹簧
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(stopButton);
    controlLayout->addStretch(); // 右侧弹簧

    // 连接信号槽
    connect(playButton, &QPushButton::clicked, this, &MediaPlayer::onPlayClicked);
    connect(pauseButton, &QPushButton::clicked, this, &MediaPlayer::onPauseClicked);
    connect(stopButton, &QPushButton::clicked, this, &MediaPlayer::onStopClicked);

    mainLayout->addWidget(controlWidget);
}

void MediaPlayer::onPlayClicked()
{
    //videoLabel->setText("播放中...");
    //QString currentText = infoTextEdit->toPlainText();
    //if (!currentText.isEmpty()) {
    //    videoLabel->setText(QString("播放中: %1").arg(currentText));
    //}

    QString playUrl = infoTextEdit->toPlainText();
    if (playUrl.isEmpty())
    {
        playUrl = "E:/code/media/BaiduSyncdisk.mp4";
        //playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000002_5?token=G9dSZrnumeb1TDSf";//2560
        //playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000006_5?token=WSGLtsoIcY7bf25L";//2880

        //playUrl = "rtsp://172.16.47.126:554/rtp/34020000001180000009_34020000001320000002_20250820091840_20250820235959_3_100000_1755652956?token=yCZGygvNedUaTiZW";
        //playUrl = "rtsp://172.16.19.69/live/test";
        //playUrl = "rtsp://admin:itc20232024@172.16.19.6:554/cam/realmonitor?channel=1&subtype=0";
        //playUrl = "rtsp://172.16.19.40:554/rtp/34020000001110000001_34020000001320000001_3?token=xCO73xOfG5uekWf4";

        infoTextEdit->setPlainText(playUrl);
    }
    m_pPlayerWidget->StartPlay(playUrl);
    m_pPlayerWidget->show();
    videoLabel->hide();
}

void MediaPlayer::onPauseClicked()
{
    //videoLabel->setText("已暂停");
}

void MediaPlayer::onStopClicked()
{
    videoLabel->show();
    m_pPlayerWidget->hide();
    m_pPlayerWidget->StopPlay();

    //videoLabel->setText("视频播放");
    progressSlider->setValue(0);
    updateTimeDisplay(0);
}

void MediaPlayer::onOpenClicked()
{
    // 打开文件对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择媒体文件",
        "",
        "媒体文件 (*.mp4 *.avi *.mkv *.mp3 *.wav);;所有文件 (*.*)"
    );

    if (fileName.isEmpty()) {
        qInfo() << "未选择文件";
        return;
    }
    // 将文件路径设置到文本编辑框
    infoTextEdit->setPlainText(fileName);

    //// 更新视频显示标签
    //QFileInfo fileInfo(fileName);
    //videoLabel->setText(QString("已选择: %1").arg(fileInfo.fileName()));
    qInfo() << "选择的文件:" << fileName;
}

void MediaPlayer::onProgressChanged(int value)
{
    updateTimeDisplay(value);
    // 这里添加实际的进度改变逻辑
}

void MediaPlayer::onTextChanged()
{
    QString text = infoTextEdit->toPlainText();

    // 如果文本是文件路径，可以做一些验证
    if (text.length() > 0) {
        // 可以在这里添加文件路径验证逻辑
        // 或者实时搜索功能
    }
}

void MediaPlayer::updateTimeDisplay(int value)
{
    // 假设总时长为5分钟（300秒）
    int totalSeconds = 300;
    int currentSeconds = value * totalSeconds / 100;

    QTime currentTime(0, currentSeconds / 60, currentSeconds % 60);
    QTime totalTime(0, totalSeconds / 60, totalSeconds % 60);

    currentTimeLabel->setText(currentTime.toString("mm:ss"));
    totalTimeLabel->setText(totalTime.toString("mm:ss"));
}