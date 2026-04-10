#include "play_widget.h"
#include "ui_play_widget.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>

PlayWidget::PlayWidget(QWidget* parent, int index)
    : QWidget(parent)
    , m_playIndex(index)
    , ui(new Ui::PlayWidget)
{
    ui->setupUi(this);

    //setWindowFlags(Qt::Tool | Qt::Window | Qt::FramelessWindowHint);
    //this->setWindowTitle(QString("PlayWidget_%1").arg(index));
    this->setMinimumSize(100, 100);
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setStyleSheet("background-color: #F0A0A0;");
    //只有调用了winId()才会真正创建窗口句柄
    qInfo() << "Create PlayWidget Handle:" << (int)winId();
}

PlayWidget::~PlayWidget()
{
    delete ui;
}
