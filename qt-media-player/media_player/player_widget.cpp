#include "player_widget.h"
#include "video_render.h"
#include "media/media_reader.h"

PlayerWidget::PlayerWidget(QWidget* parent)
    : QWidget(parent)
    //, m_pVideoRender(new VideoRender(this))
    , m_pMediaReader(new MediaReader())
{
    this->setWindowTitle("PlayerWidget");
    this->resize(850, 650);
    //设置主窗口背景颜色
    //QPalette palette;
    //palette.setColor(QPalette::Window, QColor(50, 50, 50));
    ////    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    //this->setPalette(palette);
    m_pVideoRender = new VideoRender(this);
    m_pVideoRender->show();
}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::StartPlay(const QString& uri)
{
    MediaParameter param;
    param.sUri = uri.toStdString();
    param.videoCallback = std::bind(&PlayerWidget::HandleVideoFrame, this, std::placeholders::_1);
    m_pMediaReader->Init(param);
    m_pMediaReader->Start();
}

void PlayerWidget::HandleVideoFrame(const VideoFrame& frame)
{
    //m_pVideoRender->UpdateContent(frame.videoData);
}