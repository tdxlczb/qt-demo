#include "test01_widget.h"
#include "ui_test01_widget.h"
#include <QLabel>
#include "mask_widget.h"

Test01Widget::Test01Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Test01Widget)
{
    ui->setupUi(this);
    this->resize(800,600);
    // QPalette palette;
    // palette.setColor(QPalette::Window,QColor(50, 50, 150));
    //    // palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
    // this->setPalette(palette);
    QLabel * bgLabel = new QLabel(this);
    bgLabel->setPixmap(QPixmap(":/res/bg.png"));
    bgLabel->setScaledContents(true);
    bgLabel->show();
}

Test01Widget::~Test01Widget()
{
    delete ui;
}

void Test01Widget::ShowMask(bool isShow)
{
    m_isShowMask = isShow;
    if(!m_pMaskWidget)
    {
        m_pMaskWidget = new MaskWidget(this);
    }
    if(m_pMaskWidget && isShow)
        m_pMaskWidget->show();

    if(m_pMaskWidget && !isShow)
        m_pMaskWidget->hide();
}

void Test01Widget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);

    // 获取窗口内容区域位置（去除标题栏和边框）
    QPoint contentPos = this->geometry().topLeft();

    if(m_pMaskWidget)
        m_pMaskWidget->move(contentPos);
}

void Test01Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(m_pMaskWidget)
        m_pMaskWidget->setGeometry(this->geometry());
}
