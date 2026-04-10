#include "floating_toolbar.h"
#include <QHBoxLayout>
#include <QAction>
#include <QToolButton>
#include <QToolBar>
#include <QPushButton>
#include <QProxyStyle>
#include <QDebug>

namespace {
class CustomStyle : public QProxyStyle {
public:
    CustomStyle(QStyle* style) :QProxyStyle(style) {}
    ~CustomStyle() {}
    int pixelMetric(PixelMetric m, const QStyleOption* opt = 0, const QWidget* widget = 0) const override
    {
        if (m == QStyle::PM_ToolBarExtensionExtent)
            return 60;//指定"<<"按钮宽度//生效
        if (m == QStyle::PM_SmallIconSize)
            return 60;//指定更多菜单里下拉小图标大小
        //if (m == QStyle::PM_ToolBarIconSize)
        //    return 40;//指定工具栏图标大小
        return QProxyStyle::pixelMetric(m, opt, widget);
    }
    QIcon standardIcon(StandardPixmap sp, const QStyleOption* opt, const QWidget* widget) const override
    {
        //if (sp == QStyle::SP_ToolBarHorizontalExtensionButton) {
        //    return QIcon(":res/icon/More.png");  // 替换扩展按钮图标
        //}
        return QProxyStyle::standardIcon(sp, opt, widget);
    }
};
}

#define CLICK_CALLBACK(name) std::bind(&FloatingToolbar::on_click##name, this)


FloatingToolbar::FloatingToolbar(QWidget* parent) : QWidget(parent)
{
    this->resize(500, 60);
    //this->setStyleSheet(
    //    "QWidget {"
    //    "   background-color: rgb(46, 49, 54);"
    //    "}"
    //);

    QHBoxLayout* horLayout = new QHBoxLayout(this);
    horLayout->setSpacing(0);
    horLayout->setContentsMargins(0, 0, 0, 0);

    QAction* pAction1 = new QAction(QIcon(":res/icon/PlayForward.png"), "开始播放");
    connect(pAction1, &QAction::triggered, this, [=]() {
        qDebug() << "to Play";
        });

    QAction* pAction2 = new QAction(QIcon(":res/icon/Stop.png"), "停止播放");
    connect(pAction2, &QAction::triggered, this, [=]() {
        qDebug() << "to Stop";
        });

    QAction* pAction3 = new QAction(QIcon(":res/icon/SpeedDown.png"), "慢速播放");
    connect(pAction3, &QAction::triggered, this, [=]() {
        qDebug() << "to SpeedDown";
        });

    QAction* pAction4 = new QAction(QIcon(":res/icon/SpeedUp.png"), "快速播放");
    connect(pAction4, &QAction::triggered, this, [=]() {
        qDebug() << "to SpeedUp";
        });

    QAction* pAction5 = new QAction(QIcon(":res/icon/icon1/Snap.png"), "抓图");
    connect(pAction5, &QAction::triggered, this, [=]() {
        qDebug() << "to Snap";
        });

    QAction* pAction6 = new QAction(QIcon(":res/icon/icon1/Record.png"), "录制");
    connect(pAction6, &QAction::triggered, this, [=]() {
        qDebug() << "to Record";
        });

    QAction* pAction7 = new QAction(QIcon(":res/icon/icon1/SoundOn.png"), "声音开关");
    connect(pAction7, &QAction::triggered, this, [=]() {
        qDebug() << "to SoundOnOff";
        });

    QAction* pAction8 = new QAction(QIcon(":res/icon/icon1/Zoom.png"), "缩放");
    connect(pAction8, &QAction::triggered, this, [=]() {
        qDebug() << "to Zoom";
        });


    QToolBar* pToolBar = new QToolBar(this);
    pToolBar->addAction(pAction1);
    pToolBar->addAction(pAction2);
    pToolBar->addAction(pAction3);
    pToolBar->addAction(pAction4);
    pToolBar->addSeparator();
    pToolBar->addAction(pAction5);
    pToolBar->addAction(pAction6);
    pToolBar->addAction(pAction7);
    pToolBar->addAction(pAction8);
    //pToolBar->addAction(pActionMore);

    pToolBar->setLayoutDirection(Qt::LeftToRight);//QToolBar好像只有水平布局，垂直布局的展示一般用QMenu
    pToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);//设置只显示图标，也可以设置图标文字一起显示等其他显示方式
    //pToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pToolBar->setIconSize(QSize(40, 40)); // 统一图标大小

    //pToolBar->setMinimumSize(QSize(200, 100)); // 最小宽高
    //pToolBar->setMaximumSize(QSize(800, 400)); // 最大宽高
    //pToolBar->setFixedSize(QSize(400, 80)); // 固定宽高
    //pToolBar->layout()->setContentsMargins(5, 10, 5, 10);  // 左、上、右、下边距
    //pToolBar->setContentsMargins(5, 10, 5, 10);
    //pToolBar->layout()->setSpacing(20);  // 控件之间的间距

    auto originStyle = pToolBar->style();
    CustomStyle* pCustomStytle = new CustomStyle(originStyle);
    pToolBar->setStyle(pCustomStytle);
    /*
    * 这里个奇怪的问题，如果调用了pToolBar->setStyle，
    * 然后QToolButton设置的样式中没有设置border，
    * 后面QToolButton:hover和QToolButton:pressed的样式就不生效
    * QMenu::item的样式中没有设置border，则QMenu::item:hover样式不生效，
    * 设置了border，则QMenu::item中其他样式不生效
    */
    pToolBar->setStyleSheet(R"(
    QToolBar {
        background: rgb(50,50,150);
        spacing: 0px;
    }
    QToolBar::separator {
        background: rgb(255,0,0);
        width: 50px;
    }
    QToolButton {
        background: #9898db;
        color: #FFFFFF;
        border: 1px;
        margin: 5px;
    }
    QToolButton:hover {
        background: #3498db;
    }
    QToolButton:pressed {
        background: #ff80b9;
    }
    QMenu::item {
        background-color: #9898db;
        height: 60px;
        width: 200px;
    }
    QMenu::item:hover { 
        background-color: #3498db;
    }
    QMenu::item:pressed { 
        background-color: #ff80b9;
    }
    QToolButton#qt_toolbar_ext_button{
        qproperty-icon:url(:res/icon/More.png); 
        height: 100px
    }
    )");

    //QToolButton* extButton = pToolBar->findChild<QToolButton*>(QString("qt_toolbar_ext_button"));
    //if (extButton) {
    //    extButton->setIcon(QIcon(":res/icon/More.png"));  // 设置新图标
    //    extButton->setIconSize(QSize(80, 80));            // 调整图标大小，不生效
    //    extButton->setStyleSheet("padding: 2px;");        // 调整内边距，不生效
    //}
    qDebug() << pToolBar->styleSheet();

    horLayout->addWidget(pToolBar);//不添加布局好像就不会显示>>更多按钮
}

FloatingToolbar::~FloatingToolbar()
{
    qDebug() << "delete FloatingToolbar";
}


FloatingToolbarTest::FloatingToolbarTest(QWidget* parent)
{
    this->setWindowTitle("FloatingToolbarTest");
    this->resize(800, 600);
    //this->setStyleSheet(
    //    "QWidget {"
    //    "   background-color: #2e2c71;"
    //    "}"
    //);

    QHBoxLayout* horLayout = new QHBoxLayout(this);
    horLayout->setSpacing(0);
    horLayout->setContentsMargins(0, 0, 0, 0);

    FloatingToolbar* toolbar1 = new FloatingToolbar(this);
    horLayout->addWidget(toolbar1);
}

FloatingToolbarTest::~FloatingToolbarTest()
{

}
