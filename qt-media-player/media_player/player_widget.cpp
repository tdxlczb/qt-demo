#include "player_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include "video_render.h"
#include "audio_render.h"
#include "media/media_reader.h"
#include "media/media_display.h"
#include "fisheye_widget.h"

PlayerWidget::PlayerWidget(QWidget* parent)
    : QWidget(parent)
    , m_pAudioRender(new AudioRender())
{
    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(200, 200, 200));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);

    //auto VideoRender = new VideoRGBRender(this);
    //auto VideoRender = new PlayGLWidget(this);
    auto VideoRender = new OpenGLRenderWidget(this);
    //auto VideoRender = new SDLRenderWidget(this);
    m_pVideoRender = VideoRender;

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);  // 可选：去掉边距
    layout->setSpacing(0);                   // 可选：去掉间距
    layout->addWidget(VideoRender);

    // 设置子 widget 样式（可选）
    VideoRender->setStyleSheet("background-color: red;");
}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::StartPlay(const QString& url)
{
    //MediaParameter param;
    //param.url = url.toStdString();
    //param.hwDeviceName = "dxva2";
    ////param.hwDeviceName = "d3d11va";
    //param.outputVideoSpec = { 0, 0, AV_PIX_FMT_NONE };
    ////param.outputVideoSpec = { 0, 0, AV_PIX_FMT_RGB24 };
    //param.outputAudioSpec = { 16000, 16, 2, AV_SAMPLE_FMT_S16 };

    PlayOptions opt;
    opt.hwdevice = "dxva2";

    if (!m_pMediaReader)
        m_pMediaReader = new MediaReader(0);

    //m_pAudioRender->Start(param.outputAudioSpec, 1024);
    //m_pAudioRender->SetPCMCallback(std::bind(&MediaReader::GetAudioFrame, m_pMediaReader, std::placeholders::_1, std::placeholders::_2));

    m_pMediaReader->SetPlayEvent(this);
    m_pMediaReader->Play(url.toStdString(), opt);

    //m_pFishEyeWidget = new FishEyeWidget();
    //m_pFishEyeWidget->resize(1000, 800);
    //m_pFishEyeWidget->show();
    //m_pFishEyeWidget->SetFishEyeType(FECSetupType::None, FECShowType::Normal);

}

void PlayerWidget::StopPlay()
{
    if (m_pMediaReader)
    {
        m_pMediaReader->Stop();
    }
    if (m_pVideoRender)
        m_pVideoRender->ClearContent();
}

#include <chrono>
void PlayerWidget::onVideoFrame(const VideoFrame& frame)
{
    m_isVideoPlaying = true;

    //if (!m_pVideoConverter) {
    //    VideoSpec spec;
    //    spec = { 0, 0, AV_PIX_FMT_RGB24 };
    //    m_pVideoConverter = new VideoConverter(0, spec);
    //}
    if (m_pVideoConverter) {
        VideoFrame outFrame;
        m_pVideoConverter->DisplayInput(frame, outFrame);
        m_pVideoRender->UpdateContent(outFrame);
    }
    else {
        m_pVideoRender->UpdateContent(frame);
    }
    //auto t1 = std::chrono::high_resolution_clock().now().time_since_epoch();
    //if (m_pFishEyeWidget) {
    //    cv::Mat matIn = cv::Mat(frame.spec.height, frame.spec.width, CV_8UC3, frame.data);//传递处理后的效果图
    //    m_pFishEyeWidget->UpdateContent(matIn);
    //}
    //auto t2 = std::chrono::high_resolution_clock().now().time_since_epoch();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    //qDebug() << "delta time:" << duration;
}

void PlayerWidget::onClose(const PlayError& error)
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
    //if (m_pMediaReader)
    //    m_pMediaReader->UpdateDisplaySize(event->size().width(), event->size().height());
}