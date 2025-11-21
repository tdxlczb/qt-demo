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

const int kVideoRGBConverter = 1;
const int kAudioS16Converter = 1;

PlayerWidget::PlayerWidget(QWidget* parent, int winIndex)
    : QWidget(parent)
    , m_winIndex(winIndex)
{
    //this->setAutoFillBackground(true);
    ////设置主窗口背景颜色
    //QPalette palette;
    //palette.setColor(QPalette::Window, QColor(200, 200, 200));
    ////    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    //this->setPalette(palette);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("background-color: #F0F0F0;");
    //this->setStyleSheet("border: 2px solid red; background-color: #F0F0F0;");

    //auto VideoRender = new VideoRGBRender(this);
    //auto VideoRender = new PlayGLWidget(this);
    auto VideoRender = new OpenGLRenderWidget(this);//render背景颜色#808080
    //auto VideoRender = new SDLRenderWidget(this);
    m_pVideoRender = VideoRender;
    m_pAudioRender = new AudioRender(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);  // 设置边距是为了选中时可以设置border
    layout->setSpacing(0);                   // 可选：去掉间距
    layout->addWidget(VideoRender);
}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::StartPlay(const QString& url, int decodeType)
{
    PlayOptions opt;
    if (decodeType == 1) {
        opt.hwdevice = "dxva2";
    }
    else if(decodeType == 2) {
        opt.hwdevice = "d3d11va";
    }

    if (!m_pMediaReader)
        m_pMediaReader = new MediaReader(m_winIndex);

    m_pMediaReader->SetPlayEvent(this);
    m_pMediaReader->Play(url.toStdString(), opt);

    //m_pFishEyeWidget = new FishEyeWidget();
    //m_pFishEyeWidget->resize(1000, 800);
    //m_pFishEyeWidget->show();
    //m_pFishEyeWidget->SetFishEyeType(FECSetupType::None, FECShowType::Normal);

    if (!m_hashVideoConverter.contains(kVideoRGBConverter)) {
        VideoSpec spec;
        spec = { 0, 0, AV_PIX_FMT_RGB24 };
        VideoConverter* converter = new VideoConverter(0, spec);
        m_hashVideoConverter.insert(kVideoRGBConverter,converter);
    }
    AudioSpec spec;
    spec.sampleRate = 16000;
    spec.bitPerSample = 16;
    spec.channels = 2;
    spec.format = AV_SAMPLE_FMT_S16;
    if (!m_hashAudioConverter.contains(kAudioS16Converter)) {
        AudioConverter* converter = new AudioConverter(0, spec);
        m_hashAudioConverter.insert(kAudioS16Converter, converter);
    }

    m_pAudioRender->Start(spec, 1024);
    //m_pAudioRender->SetPCMCallback(std::bind(&MediaReader::GetAudioFrame, m_pMediaReader, std::placeholders::_1, std::placeholders::_2));
}

void PlayerWidget::StopPlay()
{
    if (m_pMediaReader)
    {
        m_pMediaReader->SetPlayEvent(nullptr);
        m_pMediaReader->Stop();
    }
    if (m_pVideoRender)
        m_pVideoRender->ClearContent();

    if (m_pAudioRender)
        m_pAudioRender->Stop();
}

void PlayerWidget::ChangeSpeed(double speed)
{
    if (m_pMediaReader) {
        m_pMediaReader->Speed(speed);
    }
}

void PlayerWidget::SeekPercent(int value)
{
    if (m_pMediaReader) {
        int seekSeconds = m_pMediaReader->GetDuration() * value / 100;
        m_pMediaReader->Seek(seekSeconds);
    }
}

#include <chrono>
void PlayerWidget::onVideoFrame(const VideoFrame& frame)
{
    m_isVideoPlaying = true;

    VideoFrame outFrame;
    if (frame.spec.format != kVideoFmtYUV420P && frame.spec.format != kVideoFmtYUVJ420P && frame.spec.format != kVideoFmtNV12) {
        auto converter = m_hashVideoConverter.value(kVideoRGBConverter);
        if (converter)
            converter->DisplayInput(frame, outFrame);
    }
    else {
        outFrame = frame;
    }

    m_pVideoRender->UpdateContent(outFrame);
    if (m_pMediaReader) {
        emit sig_PlayTime((int)outFrame.pts, m_pMediaReader->GetDuration());
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

void PlayerWidget::onAudioFrame(const AudioFrame& frame)
{
    m_isAudioPlaying = true;

    AudioFrame outFrame;
    if (frame.spec.format != kAudioFmtS16 || frame.spec.sampleRate != 16000 || frame.spec.channels != 2) {
        auto converter = m_hashAudioConverter.value(kAudioS16Converter);
        if (converter)
            converter->DisplayInput(frame, outFrame);
    }
    else {
        outFrame = frame;
    }
    m_pAudioRender->Write(outFrame);
}

void PlayerWidget::onClose(const PlayError& error)
{

}

void PlayerWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit sig_Selected(this);
    }
    QWidget::mousePressEvent(event);
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