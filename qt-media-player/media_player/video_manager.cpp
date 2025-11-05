#include "video_manager.h"
#include "ui_video_manager.h"

VideoManager::VideoManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoManager)
{
    ui->setupUi(this);
    setStyleSheet("background-color: #000000;");
    setAttribute(Qt::WA_StyledBackground, true);//启用样式表背景
    m_pGridLayout = new QGridLayout(this);
    m_pGridLayout->setSpacing(1);
}

VideoManager::~VideoManager()
{
    delete ui;
}
