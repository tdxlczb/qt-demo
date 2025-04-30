#include "dynamic_toolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QDebug>
#include <QToolButton>
#include <QProxyStyle>

class ToolBarStyle : public QProxyStyle {
public:
    QIcon standardIcon(StandardPixmap sp, const QStyleOption* opt, const QWidget* widget) const override {
        if (sp == QStyle::SP_ToolBarHorizontalExtensionButton) {
            return QIcon(":res/icon/More.png");  // 替换扩展按钮图标
        }
        return QProxyStyle::standardIcon(sp, opt, widget);
    }
};

#include <QProxyStyle>
class CustomStyle : public QProxyStyle
{
public:
    CustomStyle()
    {
        _toolBarExtendButtonIcon = QIcon(":res/icon/More.png");
    }
    ~CustomStyle()
    {
    }
    int pixelMetric(PixelMetric m, const QStyleOption* opt = 0, const QWidget* widget = 0) const
    {
        if (m == QStyle::PM_ToolBarExtensionExtent)
            return 100;//指定"《"按钮大小
        if (m == QStyle::PM_SmallIconSize)
            return 60;//指定下拉小图标大小
        return QProxyStyle::pixelMetric(m, opt, widget);
    }
private:
    QIcon _toolBarExtendButtonIcon;
};


DynamicToolBar::DynamicToolBar(QWidget *parent) : QWidget(parent)
{
    this->resize(800,600);
    this->setWindowTitle("DynamicToolBar");
    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window,QColor(50,150,150));
    this->setPalette(palette);

    QVBoxLayout * vBoxLayout = new QVBoxLayout(this);
    QHBoxLayout * hBoxLayout = new QHBoxLayout(this);

    QPushButton * btn;
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
    pToolBar->addAction(pAction5);
    pToolBar->addAction(pAction6);
    pToolBar->addAction(pAction7);
    pToolBar->addAction(pAction8);
    //pToolBar->addAction(pActionMore);

    //pToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);//图标文字一起显示
    pToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pToolBar->setIconSize(QSize(80, 80)); // 统一图标大小
    pToolBar->setMinimumHeight(80);       // 最小高度
    pToolBar->setMinimumWidth(80);       // 最小高度
    CustomStyle* pCustomStytle = new CustomStyle();
    pToolBar->setStyle(pCustomStytle);
    pToolBar->setStyleSheet("QToolBar{background:#3D3D3D;} QMenu::item{ height:60px;width:120px; padding-left:10px;} QMenu::item:pressed { background-color: rgb(32,128,247);color:#FFFFFF }");
    //pToolBar->setStyleSheet(R"(
    //QToolBar::extension {
    //    background: none;
    //    image: url(:res/icon/More.png);
    //    width: 24px;
    //    height: 24px;
    //})");
    //QToolButton* extButton = pToolBar->findChild<QToolButton*>(QString(), Qt::FindDirectChildrenOnly);
    //if (extButton) {
    //    extButton->setIcon(QIcon(":res/icon/More.png"));  // 设置新图标
    //    extButton->setIconSize(QSize(80, 80));                // 调整图标大小
    //    extButton->setStyleSheet("padding: 2px;");            // 调整内边距
    //}
    //pToolBar->setStyleSheet("QToolBar{background:#3D3D3D;} QToolButton#qt_toolbar_ext_button{qproperty-icon:url(:res/icon/More.png); width:200px;}");
    //pToolBar->setStyleSheet("QToolBar::extension { image: url(:res/icon/More.png); }");
    qDebug() << pToolBar->styleSheet();
    // 
    //auto* ptr_ext_btn = pToolBar->findChild<QToolButton*>("qt_toolbar_ext_button");
    ////ptr_ext_btn->setToolTip(QObject::tr("更多")); // tip:更多
    //ptr_ext_btn->setIcon(QIcon(":res/icon/More.png"));
    //ptr_ext_btn->setFixedSize(80,80);
    // 
    // 更多按钮显示样式, 自测过程发现写在qss中不生效，使用QIcon代替
    //QIcon extbutton_icon;
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_nor.svg"), QIcon::Normal);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_horver.svg"), QIcon::Active);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_horver.svg"), QIcon::Selected);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_disr.svg"), QIcon::Disabled);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_nor.svg"), QIcon::Normal, QIcon::On);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_horver.svg"), QIcon::Active, QIcon::On);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_horver.svg"), QIcon::Selected, QIcon::On);
    //extbutton_icon.addPixmap(QPixmap(":/playback/Resources/pic/Playback/ToolIcon/icon_tool_more_dis.svg"), QIcon::Disabled, QIcon::On);

    hBoxLayout->addWidget(pToolBar);

    vBoxLayout->addStretch();
    vBoxLayout->addLayout(hBoxLayout);
    vBoxLayout->addStretch();
}

DynamicToolBar::~DynamicToolBar()
{

}
