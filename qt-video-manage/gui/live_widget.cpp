#include "live_widget.h"
#include "ui_live_widget.h"

LiveWidget::LiveWidget(QWidget *parent, int iWidgetIndex)
    : BaseLiveWidget(parent,iWidgetIndex)
    , ui(new Ui::LiveWidget)
    , m_bgLabel(new QLabel(this))
{
    ui->setupUi(this);
//    this->resize(800,600);
    this->setWindowTitle(QString("LiveWidget#%1").arg(m_iWidgetIndex));

    //设置主窗口背景颜色
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(50, 50, 50));
    //    palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    this->setPalette(palette);

    m_bgLabel->setPixmap(QPixmap(":/res/image/logo.png"));
    m_bgLabel->setScaledContents(true);
    m_bgLabel->show();

    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

LiveWidget::~LiveWidget()
{
    delete ui;
}


void LiveWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_bgLabel->resize(this->size());

}
