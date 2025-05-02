#ifndef BASEMENU_H
#define BASEMENU_H

#include <QMenu>
#include <QWidget>
//#include "common/CommonDef.h"

//给普通菜单进行样式设置
#define SET_MENU_ATTR(MENU_PTR,STYLE_STR) \
    MENU_PTR->setStyleSheet(STYLE_STR);\
    MENU_PTR->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);\
    MENU_PTR->setAttribute(Qt::WA_TranslucentBackground);\
    MENU_PTR->setMinimumWidth(200);

//给带有勾选状态图标的菜单进行样式设置
#define SET_CHECK_MENU_ATTR(MENU_PTR,STYLE_STR) \
    MENU_PTR->setStyleSheet(STYLE_STR+\
    QString(\
    "QMenu::icon{ subcontrol-position: center right; padding-right:20;}"\
    "QMenu::item { padding-left:10;}"\
    ));\
    MENU_PTR->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);\
    MENU_PTR->setAttribute(Qt::WA_TranslucentBackground);\
    MENU_PTR->setMinimumWidth(200);

//菜单资源字符串前缀
#define RES_MENU_PRE_STR ":/iconMenu/iconMenu/"

//新增一个带图标和槽的action，他根据FLAG而初始化不同的文本和处理槽
#define ADD_ACTION(MENU_PTR,PIX,FLAG,ON_TXT,OFF_TXT,CLASS_PTR,ON_SLOT,OFF_SLOT)\
    MENU_PTR->addAction(QIcon(RES_MENU_PRE_STR PIX),FLAG?tr(ON_TXT):tr(OFF_TXT),CLASS_PTR,FLAG?ON_SLOT:OFF_SLOT);

class LiveWidget;
///
/// \brief The BaseMenu class 该类是一切右键菜单的抽象根类，一切右键菜单应从此处继承
///
class BaseMenu : public QMenu
{
    Q_OBJECT

public:
    explicit BaseMenu(LiveWidget *parent);
    virtual ~BaseMenu();
    //将菜单显示在鼠标位置上
    void ShowWithCursor();
    //获取菜单通用样式
    static const QByteArray&  GetMenuStyle();
signals:
    ///
    /// \brief 通知：向进程发送响应
    /// \param szMethod 响应方法名
    /// \param jsonObjRsp 响应内容
    /// \param szDesc 描述信息
    /// \param iCode 响应成功码
    ///
    void sig_SendJsonToClient(const QString& szMethod, QJsonObject& jsonObjRsp,const QString &szDesc,const int &iCode = 200);

protected slots:
    //通用槽:用于支持部分菜单项的勾选图标显示/隐藏
    void on_check(bool isChecked);
protected:
    LiveWidget * m_pLiveWidget;//指向该右键菜单所属的播放窗体
};

#endif // BASEMENU_H
