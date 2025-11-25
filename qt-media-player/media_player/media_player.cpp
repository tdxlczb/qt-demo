#include "media_player.h"
#include "ui_media_player.h"
#include <QTime>
#include <QStyle>
#include <QFileDialog>
#include <QResizeEvent>
#include <QDebug>
#include "player_widget.h"

QString secondsToHms(int totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    if (hours > 0)
        return QTime(hours, minutes, seconds).toString("hh:mm:ss");
    else
        return QTime(hours, minutes, seconds).toString("mm:ss");
}

VideoPlayer::VideoPlayer(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::VideoPlayer)
    , m_pPlayerWidget(new PlayerWidget(this))
{
    ui->setupUi(this);

    setupUI();
    setWindowTitle("VideoPlayer");
    setMinimumSize(800, 600); // 设置最小尺寸
    resize(800, 600);

    connect(m_pPlayerWidget, &PlayerWidget::sig_PlayTime, this, &VideoPlayer::onPlayTime, Qt::QueuedConnection); //跨线程需要使用队列模式
}

VideoPlayer::~VideoPlayer()
{
    delete ui;
}

void VideoPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

bool VideoPlayer::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == progressSlider && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            // 简化计算，直接根据比例计算
            auto pos = mouseEvent->pos();

            int value = 0;
            if (progressSlider->orientation() == Qt::Horizontal) {
                value = progressSlider->minimum() +
                    ((progressSlider->maximum() - progressSlider->minimum()) * pos.x()) /
                    progressSlider->width();
            }
            else {
                value = progressSlider->minimum() +
                    ((progressSlider->maximum() - progressSlider->minimum()) *
                        (progressSlider->height() - pos.y())) / progressSlider->height();
            }
            value = qBound(progressSlider->minimum(), value, progressSlider->maximum());
            qInfo() << "slider clicked:" << value;
            progressSlider->setValue(value);
            PlaySeek(value);
            //return true; // 事件已处理，阻止默认行为
        }
    }
    return QObject::eventFilter(obj, event);
}

void VideoPlayer::setupUI()
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

void VideoPlayer::setupVideoArea()
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

void VideoPlayer::setupProgressArea()
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
    progressSlider->setPageStep(1);//不设置好像默认为10，会出现点击进度条跳转太大
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
        "    width: 20px;"
        "    height: 20px;"
        "    margin: -8px 0;"
        "    border-radius: 8px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #3498db;"
        "    border-radius: 3px;"
        "}"
    );

    progressSlider->installEventFilter(this);

    // 总时间标签
    totalTimeLabel = new QLabel("00:00");
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
    connect(progressSlider, &QSlider::valueChanged, this, &VideoPlayer::onProgressChanged);
    connect(progressSlider, &QSlider::sliderPressed, this, &VideoPlayer::onSliderPressed);
    connect(progressSlider, &QSlider::sliderMoved, this, &VideoPlayer::onSliderMoved);
    connect(progressSlider, &QSlider::sliderReleased, this, &VideoPlayer::onSliderReleased);

    mainLayout->addWidget(progressWidget);
}

void VideoPlayer::setupInfoArea()
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
    connect(openButton, &QPushButton::clicked, this, &VideoPlayer::onOpenClicked);
    connect(infoTextEdit, &QTextEdit::textChanged, this, &VideoPlayer::onTextChanged);

    mainLayout->addWidget(infoWidget);
}

void VideoPlayer::setupControlArea()
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

    comboBox = new QComboBox();
    comboBox->addItem("0.5x");
    comboBox->addItem("1.0x");
    comboBox->addItem("1.5x");
    comboBox->addItem("2.0x");
    comboBox->setCurrentIndex(1);
    comboBox->setFixedSize(60, 30);
    comboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: #27ae60;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "}"
    );

    // 添加到布局
    controlLayout->addStretch(); // 左侧弹簧
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(stopButton);
    controlLayout->addWidget(comboBox);
    controlLayout->addStretch(); // 右侧弹簧

    // 连接信号槽
    connect(playButton, &QPushButton::clicked, this, &VideoPlayer::onPlayClicked);
    connect(pauseButton, &QPushButton::clicked, this, &VideoPlayer::onPauseClicked);
    connect(stopButton, &QPushButton::clicked, this, &VideoPlayer::onStopClicked);

    // 绑定currentIndexChanged信号 - 选项改变时触发
    QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int index) {
            qDebug() << "选项改变，当前索引:" << index;
            if (m_pPlayerWidget) {
                double speed = index * 0.5 + 0.5;
                m_pPlayerWidget->ChangeSpeed(speed);
            }
        });

    mainLayout->addWidget(controlWidget);
}

void VideoPlayer::onPlayClicked()
{
    //videoLabel->setText("播放中...");
    //QString currentText = infoTextEdit->toPlainText();
    //if (!currentText.isEmpty()) {
    //    videoLabel->setText(QString("播放中: %1").arg(currentText));
    //}

    QString playUrl = infoTextEdit->toPlainText();
    if (playUrl.isEmpty())
    {
        //playUrl = "E:/code/media/BaiduSyncdisk2.mp4";
        //playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000002_5?token=G9dSZrnumeb1TDSf";//2560
        //playUrl = "rtsp://172.16.19.44:554/rtp/34020000001180000195_34020000001310000006_5?token=WSGLtsoIcY7bf25L";//2880

        //playUrl = "rtsp://172.16.47.126:554/rtp/34020000001180000009_34020000001320000002_20250820091840_20250820235959_3_100000_1755652956?token=yCZGygvNedUaTiZW";
        playUrl = "rtsp://127.0.0.1/live/rtsp_push";
        //playUrl = "rtsp://admin:itc20232024@172.16.19.6:554/cam/realmonitor?channel=1&subtype=0";
        //playUrl = "rtsp://172.16.19.40:554/rtp/34020000001110000001_34020000001320000001_3?token=xCO73xOfG5uekWf4";

        infoTextEdit->setPlainText(playUrl);
    }
    m_pPlayerWidget->StartPlay(playUrl);
    m_pPlayerWidget->show();
    videoLabel->hide();
}

void VideoPlayer::onPauseClicked()
{
    //videoLabel->setText("已暂停");
    m_pPlayerWidget->PlayPause();
}

void VideoPlayer::onStopClicked()
{
    videoLabel->show();
    m_pPlayerWidget->hide();
    m_pPlayerWidget->StopPlay();

    //videoLabel->setText("视频播放");
    progressSlider->setValue(0);
}

void VideoPlayer::onOpenClicked()
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

void VideoPlayer::onProgressChanged(int value)
{
    // 这里添加实际的进度改变逻辑
    //qInfo() << "slider changed:" << value;

}

void VideoPlayer::onSliderPressed()
{
    qInfo() << "slider pressed";
    m_sliderDragging = true;
}

void VideoPlayer::onSliderMoved(int value)
{
    qInfo() << "slider moved:" << value;

}

void VideoPlayer::onSliderReleased()
{
    int value = progressSlider->value();
    qInfo() << "slider released:" << value;
    PlaySeek(value);
    m_sliderDragging = false;
}

void VideoPlayer::onTextChanged()
{
    QString text = infoTextEdit->toPlainText();

    // 如果文本是文件路径，可以做一些验证
    if (text.length() > 0) {
        // 可以在这里添加文件路径验证逻辑
        // 或者实时搜索功能
    }
}

void VideoPlayer::onPlayTime(int64_t seconds, int64_t totalSeconds)
{
    currentTimeLabel->setText(secondsToHms(seconds));
    totalTimeLabel->setText(secondsToHms(totalSeconds));
    if (totalSeconds > 0) {
        int percent = seconds * 100 / totalSeconds;
        if (!m_sliderDragging) {
            //qInfo() << "slider set:" << percent;
            progressSlider->setValue(percent);
        }
    }
}

void VideoPlayer::PlaySeek(int value)
{
    if (m_pPlayerWidget)
        m_pPlayerWidget->SeekPercent(value);
}
