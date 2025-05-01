#include "custom_toolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QDebug>
#include <QToolButton>
#include <QProxyStyle>

class CustomStyle : public QProxyStyle {
public:
    CustomStyle(QStyle* style) :QProxyStyle(style) {}
    ~CustomStyle() {}
    int pixelMetric(PixelMetric m, const QStyleOption* opt = 0, const QWidget* widget = 0) const override
    {
        if (m == QStyle::PM_ToolBarExtensionExtent)
            return 60;//指定"<<"按钮大小//生效
        if (m == QStyle::PM_SmallIconSize)
            return 40;//指定更多菜单里下拉小图标大小
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

CustomToolBar::CustomToolBar(QWidget *parent) : QWidget(parent)
{
    this->resize(800,600);
    this->setWindowTitle("CustomToolBar");
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window,QColor(250,250,250));
    this->setPalette(palette);

    QHBoxLayout* hBoxLayout = new QHBoxLayout(this);

    QAction * pAction1 = new QAction();
    pAction1->setIcon(QIcon(":res/icon/Up.png"));
    pAction1->setText("向上冲呀");
    pAction1->setToolTip("向上冲呀");
    connect(pAction1, &QAction::triggered, this, [=](){
        qDebug()<<"to Up";
    });
    QAction * pAction2 = new QAction(QIcon(":res/icon/Down.png"),"向下冲呀");
    connect(pAction2, &QAction::triggered, this, [=](){
        qDebug()<<"to Down";
    });
    QAction * pAction3 = new QAction(QIcon(":res/icon/Left.png"),"向左冲呀");
    connect(pAction3, &QAction::triggered, this, [=](){
        qDebug()<<"to Left";
    });
    QAction * pAction4 = new QAction(QIcon(":res/icon/Right.png"),"向右冲呀");
    connect(pAction4, &QAction::triggered, this, [=](){
        qDebug()<<"to Right";
    });
    QAction * pAction5 = new QAction(QIcon(":res/icon/UpLeft.png"),"向左上冲");
    connect(pAction5, &QAction::triggered, this, [=](){
        qDebug()<<"to UpLeft";
    });
    QAction * pAction6 = new QAction(QIcon(":res/icon/UpRight.png"),"向右上冲");
    connect(pAction6, &QAction::triggered, this, [=](){
        qDebug()<<"to UpRight";
    });
    QAction * pAction7 = new QAction(QIcon(":res/icon/DownLeft.png"),"向左下冲");
    connect(pAction7, &QAction::triggered, this, [=](){
        qDebug()<<"to DownLeft";
    });
    QAction * pAction8 = new QAction(QIcon(":res/icon/DownRight.png"),"向右下冲");
    connect(pAction8, &QAction::triggered, this, [=](){
        qDebug()<<"to DownRight";
    });
    //QAction * pActionMore = new QAction(QIcon(":res/icon/More.png"),"更多");
    //connect(pActionMore, &QAction::triggered, this, [=]()
    //{
    //    qDebug()<<"to More";
    //    QMenu * pMenu = new QMenu();
    //    pMenu->addAction(pAction1);
    //    pMenu->addAction(pAction2);
    //    pMenu->addAction(pAction3);
    //    pMenu->addAction(pAction4);
    //    pMenu->move(QCursor::pos());
    //    pMenu->exec();
    //});

    QToolBar *pToolBar = new QToolBar(this);
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
    pToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);//设置只显示图标，也可以设置图标文字一起显示等其他显示方式
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
        width: 80px;
        height: 80px;
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
        height: 80px;
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
    }
    )");

    //QToolButton* extButton = pToolBar->findChild<QToolButton*>(QString("qt_toolbar_ext_button"));
    //if (extButton) {
    //    extButton->setIcon(QIcon(":res/icon/More.png"));  // 设置新图标
    //    extButton->setIconSize(QSize(80, 80));            // 调整图标大小，不生效
    //    extButton->setStyleSheet("padding: 2px;");        // 调整内边距，不生效
    //}
    qDebug() << pToolBar->styleSheet();

    hBoxLayout->addWidget(pToolBar);//不添加布局好像就不会显示>>更多按钮
}

