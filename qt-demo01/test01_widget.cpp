#include "test01_widget.h"
#include "ui_test01_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "mask_widget.h"
#include "custom_child_widget.h"

Test01Widget::Test01Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Test01Widget)
{
    ui->setupUi(this);
    this->resize(960,540);
    this->setWindowTitle("Test01Widget");

    QLabel * bgLabel = new QLabel(this);
    bgLabel->setPixmap(QPixmap(":/res/bg04.jpg"));
    bgLabel->setScaledContents(true);
    bgLabel->show();

    setAttribute(Qt::WA_StyledBackground, true);//启用样式表背景
    m_pTimer = new QTimer(this);
    //connect(m_pTimer, &QTimer::timeout, this, &Test01Widget::SwitchBackground);
    //m_pTimer->start(2000);

    //QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    //CustomChildWidget * pChild = new CustomChildWidget(this);
    //pChild->move(200,200);
    //pChild->show();
    //vBoxLayout->addWidget(pChild);
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

void Test01Widget::SwitchBackground()
{
    //setAttribute(Qt::WA_StyledBackground, true);//使用样式前需要启用样式表背景
    static QStringList imagePaths = {
        ":/res/bg01.jpg",  // Qt资源文件路径
        ":/res/bg02.jpg",
        ":/res/bg03.jpg",
        ":/res/bg04.jpg"
    };
    // 循环切换图片
    m_iCurrentIndex = (m_iCurrentIndex + 1) % imagePaths.size();
    QString path = imagePaths[m_iCurrentIndex];

    // 设置样式表（关键：background-size: cover 撑满窗口）
    this->setStyleSheet(
        QString("QWidget {"
            "background-image: url(%1);"
            "background-position: center;"
            "background-repeat: no-repeat;"
            "background-attachment: fixed;"
            "background-size: cover;"  // 填充整个窗口
            "}").arg(path)
    );
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
