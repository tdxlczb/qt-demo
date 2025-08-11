#include "player_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QResizeEvent>
#include <QDebug>
#include "video_render.h"
#include "audio_render.h"
#include "media/media_reader.h"

PlayerWidget::PlayerWidget(QWidget* parent)
    : QWidget(parent)
    , m_pAudioRender(new AudioRender())
{
    this->setWindowTitle("PlayerWidget");
    this->resize(850, 650);
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);

    //auto VideoRender = new VideoRGBRender(this);
    auto VideoRender = new PlayGLWidget(this);
    //auto VideoRender = new SDLRenderWidget(this);
    m_pVideoRender = VideoRender;

    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setSpacing(10);//设置间距
    vBoxLayout->setContentsMargins(10, 10, 10, 10);//设置边距
    vBoxLayout->addWidget(VideoRender);
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
    //param.hwDeviceName = "dxva2";
    param.outputVideoSpec = { 0, 0, AV_PIX_FMT_YUV420P };
    //param.outputVideoSpec = { 0, 0, AV_PIX_FMT_RGB24 };
    param.outputAudioSpec = { 16000, 16, 2, AV_SAMPLE_FMT_S16 };

    if (!m_pMediaReader)
        m_pMediaReader = new MediaReader();

    //m_pAudioRender->Start(param.outputAudioSpec, 1024);
    //m_pAudioRender->SetPCMCallback(std::bind(&MediaReader::GetAudioFrame, m_pMediaReader, std::placeholders::_1, std::placeholders::_2));

    if (m_pMediaReader->Init(param)) {
        m_pMediaReader->SetPlayEvent(this);
        m_pMediaReader->Start();
    }
}

void PlayerWidget::StopPlay()
{
    if (m_pMediaReader)
    {
        m_pMediaReader->Stop();
    }
}

void PlayerWidget::onVideoFrame(const VideoFrame& frame)
{
    m_isVideoPlaying = true;
    m_pVideoRender->UpdateContent(frame);
}

void PlayerWidget::onClose(const PlayError& error)
{

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
        //playUrl = "rtsp://172.16.47.126:554/rtp/34020000001180000002_34020000002000000002_20250724091720_20250724235959_1_100000_1753328209?token=YyC43CkbA5E8RUW7";
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
    if (m_pMediaReader)
        m_pMediaReader->UpdateDisplaySize(event->size().width(), event->size().height());
}