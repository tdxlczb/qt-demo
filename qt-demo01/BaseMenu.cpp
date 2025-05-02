#include "BaseMenu.h"

#include <QFile>
#include <QDebug>
#include "LiveWidget.h"

BaseMenu::BaseMenu(LiveWidget *parent)
    : QMenu(parent)
    ,m_pLiveWidget(parent)
{
    Q_ASSERT(m_pLiveWidget);
    connect(this,&BaseMenu::sig_SendJsonToClient,m_pLiveWidget,&LiveWidget::sig_SendJsonToClient);
}

BaseMenu::~BaseMenu()
{
}

void BaseMenu::ShowWithCursor()
{
    //如果菜单中没有东西，那么直接退出
    if(actions().size() == 0)
        return;
    move(QCursor::pos());
    this->resize(this->sizeHint().width(),this->sizeHint().height());//解决最后一项的残缺显示问题
    exec();
}

const QByteArray& BaseMenu::GetMenuStyle()
{
    static QByteArray sMenuStyle;
    if(sMenuStyle.isEmpty()){
        QFile file(":/qss/qss/menu.qss");
        if(file.open(QFile::ReadOnly)){
            sMenuStyle = file.readAll();
            file.close();
        }
    }
    return sMenuStyle;
}

void BaseMenu::on_check(bool isChecked)
{
    QAction *pAct = qobject_cast<QAction*>(sender());
    if(nullptr == pAct){
        qCritical()<<"异常情况，被更新勾选状态的Action为nullptr";
        return;
    }
    if (isChecked) pAct->setIcon(QIcon(":/icon/icon/Check.png"));
     else pAct->setIcon(QIcon());
}
