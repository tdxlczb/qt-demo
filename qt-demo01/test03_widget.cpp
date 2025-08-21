#include "test03_widget.h"
#include "ui_test03_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>


AdaptiveLabel::AdaptiveLabel(QWidget* parent)
    : QLabel(parent)
{
}

AdaptiveLabel::~AdaptiveLabel()
{
}

void AdaptiveLabel::SetAdaptiveText(const QString& szText, bool bSetFixed)
{
    m_szText = szText;
    this->setToolTip(szText);
    QString szTemp = szText;
    QFontMetrics fontMetrics(this->font());
    int textWidth = this->fontMetrics().horizontalAdvance(szText);
    int labelShowWidth = this->width();
    if (textWidth > labelShowWidth) {
        szTemp = fontMetrics.elidedText(szText, Qt::ElideRight, labelShowWidth);
    }
    this->setText(szTemp);

    int tempTextWidth = this->fontMetrics().horizontalAdvance(szTemp);
    // 设置固定宽度（或额外增加边距）
    this->setFixedWidth(tempTextWidth + 10); // +10 为预留边距
}


void AdaptiveLabel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    SetAdaptiveText(m_szText);
}


Test03Widget::Test03Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Test03Widget)
{
    ui->setupUi(this);
    this->setMinimumSize(50, 50);
    //// 创建主垂直布局
    //QVBoxLayout* mainLayout = new QVBoxLayout(this);

    //// 第一行：两个按钮
    //QHBoxLayout* firstRowLayout = new QHBoxLayout();
    //QPushButton* button1 = new QPushButton("按钮1", this);
    //QPushButton* button2 = new QPushButton("按钮2", this);
    //firstRowLayout->addWidget(button1);
    //firstRowLayout->addWidget(button2);

    //// 第二行：两个标签
    //QHBoxLayout* secondRowLayout = new QHBoxLayout();
    //QLabel* label1 = new QLabel("标签1", this);
    //QLabel* label2 = new QLabel("标签2", this);
    //secondRowLayout->addWidget(label1);
    //secondRowLayout->addWidget(label2);

    //// 第三行：两个按钮
    //QHBoxLayout* thirdRowLayout = new QHBoxLayout();
    //QPushButton* button3 = new QPushButton("按钮3", this);
    //QPushButton* button4 = new QPushButton("按钮4", this);
    //thirdRowLayout->addWidget(button3);
    //thirdRowLayout->addWidget(button4);

    //// 将所有行添加到主布局
    //mainLayout->addLayout(firstRowLayout);
    //mainLayout->addLayout(secondRowLayout);
    //mainLayout->addLayout(thirdRowLayout);

    //// 设置布局边距和间距
    //mainLayout->setContentsMargins(10, 10, 10, 10);
    //mainLayout->setSpacing(15);

    //QVBoxLayout* vBoxLayout = new QVBoxLayout(this);

    QHBoxLayout* hBoxLayout = new QHBoxLayout(this);

    AdaptiveLabel* label01 = new AdaptiveLabel(this);
    label01->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label01->setStyleSheet("QLabel { background-color: rgb(150, 0, 0); }");
    label01->SetAdaptiveText("1测试123测试123测试123测试123测试123测试123");

    AdaptiveLabel* label02 = new AdaptiveLabel(this);
    label02->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label02->setStyleSheet("QLabel { background-color: rgb(150, 150, 0); }");
    label02->SetAdaptiveText("2测试123测试123");

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    AdaptiveLabel* label03 = new AdaptiveLabel(this);
    label03->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label03->setStyleSheet("QLabel { background-color: rgb(150, 150, 150); }");
    label03->SetAdaptiveText("3测试123测试123");

    AdaptiveLabel* label04 = new AdaptiveLabel(this);
    label04->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label04->setStyleSheet("QLabel { background-color: rgb(0, 150, 0); }");
    label04->SetAdaptiveText("4测试123测试123");

    QPushButton* button = new QPushButton("按钮1", this);
    button->setFixedWidth(40);
    QPushButton* button2 = new QPushButton("按钮2", this);
    button2->setFixedWidth(40);

    hBoxLayout->addWidget(button);
    hBoxLayout->addWidget(label01);
    hBoxLayout->addWidget(label02);
    hBoxLayout->addWidget(label03);
    hBoxLayout->addWidget(label04);
    hBoxLayout->addWidget(button2);

    //vBoxLayout->addLayout(hBoxLayout);
}

Test03Widget::~Test03Widget()
{
    delete ui;
}

void Test03Widget::SetAdaptiveLabel(QLabel* label, const QString& szText)
{
    QString szTemp = szText;
    label->setToolTip(szText);
    QFontMetrics fontMetrics(label->font());
    int textWidth = label->fontMetrics().horizontalAdvance(szText);
    int labelShowWidth = label->width();
    if (textWidth > labelShowWidth) {
        szTemp = fontMetrics.elidedText(szText, Qt::ElideRight, labelShowWidth);
    }
    else {
        // 设置固定宽度（或额外增加边距）
        label->setFixedWidth(textWidth + 10); // +10 为预留边距
    }
    label->setText(szTemp);
}

void Test03Widget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}
