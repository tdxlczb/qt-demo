#include "player_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QDebug>
#include "video_render.h"
#include "audio_render.h"
#include "media/media_reader.h"

PlayerWidget::PlayerWidget(QWidget* parent)
    : QWidget(parent)
    , m_pVideoRender(new VideoRender(this))
    , m_pAudioRender(new AudioRender(this))
{
    this->setWindowTitle("PlayerWidget");
    this->resize(850, 650);
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setSpacing(10);//设置间距
    vBoxLayout->setContentsMargins(10, 10, 10, 10);//设置边距
    vBoxLayout->addWidget(m_pVideoRender);
    QProgressBar* progressBar = new QProgressBar(this);
    progressBar->setFixedHeight(20);
    vBoxLayout->addWidget(progressBar);

    QHBoxLayout* hBoxLayoutUrl = new QHBoxLayout(this);
    QPushButton* btnOpenFile = new QPushButton("打开", this);
    btnOpenFile->setFixedHeight(30);
    connect(btnOpenFile, &QPushButton::clicked, this, &PlayerWidget::on_pbOpenFileButton_clicked);
    m_pTextEditUrl = new QTextEdit(this);
    m_pTextEditUrl->setFixedHeight(30);
    m_pTextEditUrl->setPlaceholderText("请输入音视频文件路径，或者音视频流地址");
    hBoxLayoutUrl->addWidget(btnOpenFile);
    hBoxLayoutUrl->addWidget(m_pTextEditUrl);
    vBoxLayout->addLayout(hBoxLayoutUrl);

    QHBoxLayout* hBoxLayoutPlayControl = new QHBoxLayout(this);
    QPushButton* btnPlay = new QPushButton("播放", this);
    QPushButton* btnStop = new QPushButton("停止", this);
    connect(btnPlay, &QPushButton::clicked, this, &PlayerWidget::on_pbPlayButton_clicked);
    connect(btnStop, &QPushButton::clicked, this, &PlayerWidget::on_pbStopButton_clicked);
    hBoxLayoutPlayControl->addWidget(btnPlay);
    hBoxLayoutPlayControl->addWidget(btnStop);
    vBoxLayout->addLayout(hBoxLayoutPlayControl);
}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::StartPlay(const QString& url)
{
    m_playUrl = url;
    m_pTextEditUrl->setText(url);
    MediaParameter param;
    param.url = url.toStdString();
    param.hwDeviceName = "dxva2";
    param.outputSampleRate = 16000;
    param.outputBitPerSample = 16;
    param.outputChannelCount = 2;
    param.videoCallback = std::bind(&PlayerWidget::HandleVideoFrame, this, std::placeholders::_1);
    param.audioCallback = std::bind(&PlayerWidget::HandleAudioFrame, this, std::placeholders::_1);

    //m_pAudioRender->Start(param.outputSampleRate, param.outputBitPerSample, param.outputChannelCount, 1024);

    if (!m_pMediaReader)
        m_pMediaReader = new MediaReader();
    if (m_pMediaReader->Init(param))
        m_pMediaReader->Start();
}

void PlayerWidget::StopPlay()
{
    if (m_pMediaReader)
    {
        m_pMediaReader->Stop();
    }
}

void PlayerWidget::on_pbOpenFileButton_clicked()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,                       // 父窗口
        tr("选择文件"),             // 对话框标题
        QDir::homePath(),           // 默认打开的目录
        tr("所有文件 (*);;视频文件 (*.mp4);;音频文件 (*.mp3 *.wav)") // 文件过滤器
    );

    if (fileName.isEmpty()) {
        qDebug() << "未选择文件";
        return;
    }
    m_pTextEditUrl->setText(fileName);
    qDebug() << "选择的文件:" << fileName;
}

void PlayerWidget::on_pbPlayButton_clicked()
{
    QString playUrl = m_pTextEditUrl->toPlainText();
    if (playUrl.isEmpty())
    {
        playUrl = "E:/code/media/BaiduSyncdisk.mp4";
        //playUrl = "rtsp://172.16.45.151:554/rtp/34020000001180000002_34020000002000000002_1?token=Uw4fJbAgImgvvxSm";
        //playUrl = "rtsp://127.0.0.1/live/test";
        //playUrl = "rtsp://admin:itc20232024@172.16.19.6:554/cam/realmonitor?channel=1&subtype=0";
        //playUrl = "rtsp://172.16.19.44:554/proxy/44_160_0?token=uYeCn9fSppapqAbK";
    }
    StartPlay(playUrl);
}

void PlayerWidget::on_pbStopButton_clicked()
{

}

void PlayerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    //if (m_pVideoRender)
    //{
    //    QRect rc = geometry();
    //    qDebug() << "player widget geometry:(" << rc.x() << "," << rc.y() << "," << rc.width() << "," << rc.height() << ")";
    //    QRect renderRect = QRect(0, 0, rc.width() - 20, rc.height() - 120);
    //    //m_pVideoRender->setGeometry(renderRect);
    //    m_pVideoRender->resize(rc.width() - 40, rc.height() - 150);
    //    m_pVideoRender->move(10, 10);
    //}
}

void PlayerWidget::HandleVideoFrame(const VideoFrame& frame)
{
    m_isVideoPlaying = true;
    m_pVideoRender->UpdateContent(frame.videoData);
}

void PlayerWidget::HandleAudioFrame(const AudioFrame& frame)
{
    m_isAudioPlaying = true;
    m_pAudioRender->Write(frame);
}