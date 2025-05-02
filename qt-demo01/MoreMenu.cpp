#include "MoreMenu.h"
#include <QActionGroup>
#include <QFile>
#include <QDebug>
//#include "core/MediaApplication.h"
#include "LiveWidget.h"
//#include "Utils/StringUtil.h"

const QVector<QString> g_vStreamNames{
"主码流","子码流","第三码流","虚拟码流","第五码流","第六码流","第七码流","第八码流","第九码流","第十码流"
};


MoreMenu::MoreMenu(LiveWidget *parent)
    : BaseMenu(parent)
    ,m_pMenuHistory(nullptr)
    ,m_pMenuStream(nullptr)
    ,m_pActGroupStream(nullptr)
{
    Q_ASSERT(m_pLiveWidget);
//    if(qApp->GetWebInfo().vToolUniversal.contains("Universal_Talk")){
    if(false){
        if(m_pLiveWidget->IsAnyTalk()){
            this->addAction(QIcon(RES_MENU_PRE_STR"TalkStop.png"),tr("停止对讲"), this, &MoreMenu::on_stopVoiceIntercom);
        }else{
            this->addAction(QIcon(RES_MENU_PRE_STR"Talk.png"),tr("开始对讲"), this,[this](){on_startVoiceIntercom(1);});
        }
    }
//    if(qApp->GetWebInfo().vToolUniversal.contains("Universal_Zoom")){
    if(false){
        ADD_ACTION(this,"Zoom.png",m_pLiveWidget->IsZoom(),"关闭电子放大","启用电子放大",this,&MoreMenu::on_stopZoom,&MoreMenu::on_startZoom);
    }
    //if(qApp->GetWebInfo().vToolUniversal.contains("Universal_InstantPlayBack")){
    if(false){
        m_pMenuHistory =this->addMenu(QIcon(RES_MENU_PRE_STR"History.png"),tr("切换至即时回放"));
        m_pMenuHistory->addAction(tr("30秒"),this,[this](){on_instantPlayBack(30);});
        m_pMenuHistory->addAction(tr("1分钟"),this,[this](){on_instantPlayBack(1*60);});
        m_pMenuHistory->addAction(tr("3分钟"),this,[this](){on_instantPlayBack(3*60);});
        m_pMenuHistory->addAction(tr("5分钟"),this,[this](){on_instantPlayBack(5*60);});
        m_pMenuHistory->addAction(tr("8分钟"),this,[this](){on_instantPlayBack(8*60);});
        m_pMenuHistory->addAction(tr("10分钟"),this,[this](){on_instantPlayBack(10*60);});
    }
//    if(qApp->GetWebInfo().vToolUniversal.contains("Universal_Stream")){
    if(false){
        m_pMenuStream =this->addMenu(QIcon(RES_MENU_PRE_STR"Stream.png"),tr("码流"));
        for(int i=0,iMax = g_vStreamNames.size();i<iMax;++i){
            m_pMenuStream->addAction(g_vStreamNames[i],this,[this,i](){on_switchStream(false,i);});
        }
        m_pMenuStream->addAction(tr("自动改变码流类型"),this,[this](){on_switchStream(true,0);});
        m_pActGroupStream = new QActionGroup(this);
        m_pActGroupStream->setExclusive(true);
        for(auto &one:m_pMenuStream->actions()){
            m_pActGroupStream->addAction(one);
            one->setCheckable(true);
            connect(one,&QAction::toggled,this,&MoreMenu::on_check);
        }
        //if(m_pLiveWidget->GetDevInfo().isStreamAuto){
        if(true){
            if(m_pMenuStream->actions().size() == 0){
                qCritical()<<"错误：码流菜单的action数为0";
            }else{
                m_pMenuStream->actions().last()->setChecked(true);
            }
        }
        else{
//            if(m_pMenuStream->actions().size() <= m_pLiveWidget->GetDevInfo().iStreamIndex){
//                qCritical()<<"错误：码流菜单的action数为"<<m_pMenuStream->actions().size()<<"，但当前下标为"<<m_pLiveWidget->GetDevInfo().iStreamIndex;
//            }else{
//                m_pMenuStream->actions()[m_pLiveWidget->GetDevInfo().iStreamIndex]->setChecked(true);
//            }
        }
    }
    CreateHistoryMenu();
    CreateStreamMenu();
    const QByteArray &szStyle{GetMenuStyle()};
    SET_MENU_ATTR(this,szStyle);
    if(m_pMenuHistory){
        SET_MENU_ATTR(m_pMenuHistory,szStyle);
    }
    if(m_pMenuStream){
        SET_CHECK_MENU_ATTR(m_pMenuStream,szStyle);
    }
}

void MoreMenu::CreateHistoryMenu()
{
    //m_pMenuHistory = this->addMenu(QIcon(RES_MENU_PRE_STR"History.png"), tr("切换至即时回放"));
    m_pMenuHistory = new QMenu();
    m_pMenuHistory->setIcon(QIcon(":res/icon/iconMenu/History.png"));
    m_pMenuHistory->setTitle(tr("切换至即时回放"));
    m_pMenuHistory->addAction(tr("30秒"), this, [this]() {on_instantPlayBack(30); });
    m_pMenuHistory->addAction(tr("1分钟"), this, [this]() {on_instantPlayBack(1 * 60); });
    m_pMenuHistory->addAction(tr("3分钟"), this, [this]() {on_instantPlayBack(3 * 60); });
    m_pMenuHistory->addAction(tr("5分钟"), this, [this]() {on_instantPlayBack(5 * 60); });
    m_pMenuHistory->addAction(tr("8分钟"), this, [this]() {on_instantPlayBack(8 * 60); });
    m_pMenuHistory->addAction(tr("10分钟"), this, [this]() {on_instantPlayBack(10 * 60); });
}

void MoreMenu::CreateStreamMenu()
{
    //m_pMenuStream = this->addMenu(QIcon(RES_MENU_PRE_STR"Stream.png"), tr("码流"));
    m_pMenuStream = new QMenu();
    m_pMenuStream->setIcon(QIcon(":res/icon/iconMenu/Stream.png"));
    m_pMenuStream->setTitle(tr("码流"));
    for (int i = 0, iMax = g_vStreamNames.size(); i < iMax; ++i) {
        m_pMenuStream->addAction(g_vStreamNames[i], this, [this, i]() {on_switchStream(false, i); });
    }
    m_pMenuStream->addAction(tr("自动改变码流类型"), this, [this]() {on_switchStream(true, 0); });
    m_pActGroupStream = new QActionGroup(this);
    m_pActGroupStream->setExclusive(true);
    for (auto& one : m_pMenuStream->actions()) {
        m_pActGroupStream->addAction(one);
        one->setCheckable(true);
        connect(one, &QAction::toggled, this, &MoreMenu::on_check);
    }
    //if(m_pLiveWidget->GetDevInfo().isStreamAuto){
    if (true) {
        if (m_pMenuStream->actions().size() == 0) {
            qCritical() << "错误：码流菜单的action数为0";
        }
        else {
            m_pMenuStream->actions().last()->setChecked(true);
        }
    }
    else {
        //            if(m_pMenuStream->actions().size() <= m_pLiveWidget->GetDevInfo().iStreamIndex){
        //                qCritical()<<"错误：码流菜单的action数为"<<m_pMenuStream->actions().size()<<"，但当前下标为"<<m_pLiveWidget->GetDevInfo().iStreamIndex;
        //            }else{
        //                m_pMenuStream->actions()[m_pLiveWidget->GetDevInfo().iStreamIndex]->setChecked(true);
        //            }
    }
}

QMenu* MoreMenu::GetHistoryMenu()
{
    return m_pMenuHistory;
}

QMenu* MoreMenu::GetStreamMenu()
{
    return m_pMenuStream;
}

void MoreMenu::on_startVoiceIntercom(int iVoiceChannel)
{
//    QJsonObject jsnObj;
//    jsnObj.insert("talkType", (int)ETalkType::TALK);
//    jsnObj.insert("isEnable", true);
//    jsnObj.insert("voiceChannel", iVoiceChannel);
//    emit sig_SendJsonToClient("rsp.SetVoiceIntercom", jsnObj,"开始语音对讲");
}

void MoreMenu::on_startIpVoiceIntercom()
{
//    QJsonObject jsnObj;
//    jsnObj.insert("talkType", (int)ETalkType::IP_TALK);
//    jsnObj.insert("isEnable", true);
//    emit sig_SendJsonToClient("rsp.SetVoiceIntercom", jsnObj,"开始IP通道对讲");
}

void MoreMenu::on_stopVoiceIntercom()
{
//    QJsonObject jsnObj;
//    jsnObj.insert("isEnable", false);
//    emit sig_SendJsonToClient("rsp.SetVoiceIntercom", jsnObj,"停止对讲");
}

void MoreMenu::on_startZoom()
{
//    QJsonObject jsnObj;
//    jsnObj.insert("isEnable", true);
//    emit sig_SendJsonToClient("rsp.SetZoom", jsnObj,"开始电子放大");
}

void MoreMenu::on_stopZoom()
{
//    QJsonObject jsnObj;
//    jsnObj.insert("isEnable", false);
//    emit sig_SendJsonToClient("rsp.SetZoom", jsnObj,"停止电子放大");
}

void MoreMenu::on_instantPlayBack(int iSec)
{
//    QJsonObject jsnObj;
//    jsnObj.insert("timeArg", iSec);
//    jsnObj.insert("protocol", (int)m_pLiveWidget->GetDevInfo().eMediaPlayerType);
//    jsnObj.insert("digitalChan", m_pLiveWidget->GetDevInfo().iChannelId);
//    jsnObj.insert("playType", (int)m_pLiveWidget->GetDevInfo().ePlayType);
//    emit sig_SendJsonToClient("rsp.PlayBackTimeArg", jsnObj,"即时回放");
}

void MoreMenu::on_switchStream(bool isStreamAuto, int iStreamIndex)
{
    //变更设备的码流信息
//    StMediaDevInfo stDevInfo = m_pLiveWidget->GetDevInfo();
//    stDevInfo.isStreamAuto = isStreamAuto;
//    m_pLiveWidget->SetDevInfo(stDevInfo);
//    if(isStreamAuto){
//        m_pLiveWidget->CheckAutoStreamForSend();
//        return;
//    }
//    QJsonObject infoObject;
//    infoObject.insert("streamIndex", iStreamIndex + 1);//前端的主码流是1，播放端是0，所以这里要+1
//    infoObject.insert("protocol", (int)stDevInfo.eMediaPlayerType);
//    infoObject.insert("digitalChan", stDevInfo.iChannelId);
//    emit sig_SendJsonToClient("rsp.SwitchStream", infoObject,"码流切换");
}
