#include "media_manager_widget.h"
#include "ui_media_manager_widget.h"

#include "media_player/media_player.h"

MediaManagerWidget::MediaManagerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaManagerWidget)
{
    ui->setupUi(this);

    connect(ui->buttonStart, &QPushButton::clicked, this, &MediaManagerWidget::on_buttonStart_clicked);
    connect(ui->buttonStop, &QPushButton::clicked, this, &MediaManagerWidget::on_buttonStop_clicked);

    m_pMediaPlayer = new MediaPlayer(nullptr);
    m_pMediaPlayer->show();
}

MediaManagerWidget::~MediaManagerWidget()
{
    delete ui;
}

void MediaManagerWidget::on_buttonStart_clicked()
{
    QString editUrl = GetEditUrl();
    if(editUrl.isEmpty())
        return;

    if(!m_pMediaPlayer)
        return;


}

void MediaManagerWidget::on_buttonStop_clicked()
{

}

QString MediaManagerWidget::GetEditUrl()
{
    return ui->editUrl->toPlainText();
}
