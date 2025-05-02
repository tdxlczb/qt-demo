#include "LiveBottomFloatingBar.h"
#include "ui_LiveBottomFloatingBar.h"
#include <QHBoxLayout>
#include <QList>
#include <QToolButton>
#include <QActionGroup>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
//#include "core/MediaApplication.h"
//#include "gui/FadeOutPopup.h"
#include "LiveWidget.h"
//#include "guiMenu/FireMenu.h"
#include "MoreMenu.h"


#define CLICK_CALLBACK(name) std::bind(&LiveBottomFloatingBar::on_##name##_clicked, this)
const QString kToolBarIconPath = ":res/icon/icon/";
const QString kToolBarMenuIconPath = ":res/icon/iconMenu/";

LiveBottomFloatingBar::LiveBottomFloatingBar(LiveWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::LiveBottomFloatingBar)
    ,m_pLiveWidget(parent)
{
    Q_ASSERT(m_pLiveWidget);
    ui->setupUi(this);
    //链接内置，增强内聚
//    connect(m_pLiveWidget,&LiveWidget::sig_StartRecord,this,&LiveBottomFloatingBar::on_StartRecord);
//    connect(m_pLiveWidget,&LiveWidget::sig_StopRecord,this,&LiveBottomFloatingBar::on_StopRecord);
//    connect(m_pLiveWidget,&LiveWidget::sig_SetGas,this,&LiveBottomFloatingBar::on_SetGas);
//    connect(m_pLiveWidget,&LiveWidget::sig_SetMask,this,&LiveBottomFloatingBar::on_SetMask);
//    connect(m_pLiveWidget,&LiveWidget::sig_UpdateConfig,this,&LiveBottomFloatingBar::on_UpdateConfig);

    initControl();
    //基于当前全局配置进行初始化
    StWebInfo stWebInfo;
    on_UpdateConfig(stWebInfo);

    qInfo()<<"LiveBottomFloatingBar created.";
}

LiveBottomFloatingBar::~LiveBottomFloatingBar()
{
    delete ui;
    qInfo()<<"LiveBottomFloatingBar removed.";
}

const QString kNameSnap = "Universal_Snap";
const QString kNameRecord = "Universal_Record";
const QString kNameTalk = "Universal_Talk";
const QString kNameZoom = "Universal_Zoom";
const QString kNameInstantPlayBack = "Universal_InstantPlayBack";
const QString kNameStream = "Universal_Stream";

const QString kNameFEC = "Dedicated_FEC";
const QString kNameFireFrame = "Dedicated_FireFrame";
const QString kNameGas = "Dedicated_Gas";
const QString kNameMask = "Dedicated_Mask";
const QString kNameWarnOutCtrl = "Dedicated_WarnOutCtrl";

void LiveBottomFloatingBar::initControl()
{
    m_listUniversalTools = {
        { kNameSnap,             "Snap.png",     tr("抓图"),           CLICK_CALLBACK(pbSnap) },
        { kNameRecord,           "Record.png",   tr("开始录像"),       CLICK_CALLBACK(pbRecord) },
        { kNameTalk,             "Talk.png",     tr("开始对讲"),       CLICK_CALLBACK(pbTalk) },
        { kNameZoom,             "Zoom.png",     tr("启用电子放大"),   CLICK_CALLBACK(pbZoom) },
        { kNameInstantPlayBack,  "History.png",  tr("切换至即时回放"),  CLICK_CALLBACK(pbInstantPlayBack) },
        { kNameStream,           "Stream.png",   tr("码流"),           CLICK_CALLBACK(pbStream) }
    };

    m_listDedicatedTools = {
        { kNameFEC,          "Fish.png",         tr("鱼眼展开"),     CLICK_CALLBACK(pbFEC) },
        { kNameFireFrame,    "FireMsg.png",      tr("火点信息"),     CLICK_CALLBACK(pbFireFrame) },
        { kNameGas,          "Gas.png",          tr("显示废气信息"), CLICK_CALLBACK(pbGas) },
        { kNameMask,         "Mask.png",         tr("显示屏蔽区域"), CLICK_CALLBACK(pbMask) },
        { kNameWarnOutCtrl,  "WarnOutCtrl.png",  tr("报警输出控制"), CLICK_CALLBACK(pbWarnOutCtrl) },
    };

    m_pMoreMenu = new MoreMenu(m_pLiveWidget);
}

ToolsInfo LiveBottomFloatingBar::getToolsInfo(const QString& toolName)
{
	ToolsInfo info;
	auto it = std::find_if(m_listDedicatedTools.begin(), m_listDedicatedTools.end(), [toolName](const ToolsInfo& item) {
		return item.sName == toolName;
		});
	if (it != m_listDedicatedTools.end())
	{
		info = *it;
	}
	else
	{
		auto it1 = std::find_if(m_listUniversalTools.begin(), m_listUniversalTools.end(), [toolName](const ToolsInfo& item) {
			return item.sName == toolName;
			});
		if (it1 != m_listUniversalTools.end())
		{
			info = *it1;
		}
	}
	return info;
}

void LiveBottomFloatingBar::on_UpdateConfig(const StWebInfo &stWebInfo)
{
    //配置底部栏的按钮显隐/排列
    //QJsonArray{"Snap","Record","Talk","Zoom","InstantPlayBack","Stream"};
    //QJsonArray{"FEC","SmartTrack","FireFrame","PTZP","TempTest","FireAlarm","LACP","Mask","Gas","LockCtrl","WarnOutCtrl"};

    QList<QString> listUniversalTools = { "Universal_Snap" ,"Universal_Record" ,"Universal_Talk" ,"Universal_Zoom" ,"Universal_InstantPlayBack","Universal_Stream" };
    QList<QString> listDedicatedTools = { "Dedicated_FEC" ,"Dedicated_FireFrame" ,"Dedicated_Gas" ,"Dedicated_Mask" ,"Dedicated_WarnOutCtrl" };

    //QList<QString> listUniversalTools = { "Universal_Snap" ,"Universal_Record"  };
    //QList<QString> listDedicatedTools = { "Dedicated_FEC" ,"Dedicated_FireFrame"  };

    m_currentUniversalTools.clear();
    m_currentDedicatedTools.clear();

    ui->horizontalLayout->addStretch();

    //先显示DedicatedTools，再显示UniversalTools
    int btnIndex = 0;
    for (int i = 0; i < listDedicatedTools.size(); i++)
    {
        auto toolName = listDedicatedTools[i];
        auto it = std::find_if(m_listDedicatedTools.begin(), m_listDedicatedTools.end(), [toolName](const ToolsInfo& item) {
            return item.sName == toolName;
            });
        if (it != m_listDedicatedTools.end()) {

            QAction* pAction = new QAction();
            pAction->setIcon(QIcon(QString(kToolBarIconPath + m_listDedicatedTools[i].sIconName)));
            pAction->setToolTip(m_listDedicatedTools[i].sText);
            pAction->setText(m_listDedicatedTools[i].sText);
            pAction->setObjectName(m_listDedicatedTools[i].sName);
            connect(pAction, &QAction::triggered, this, [=]() {
                m_listDedicatedTools[i].clickedCallback();
                });
            m_hashToolsAction.insert(m_listDedicatedTools[i].sName, pAction);

            QToolButton* pBtn = new QToolButton(this);
            pBtn->setObjectName(m_listDedicatedTools[i].sName);
            pBtn->setDefaultAction(pAction);
            btnIndex++;
            ui->horizontalLayout->addWidget(pBtn);
        }
        m_currentDedicatedTools.push_back(toolName);
    }
    if (listDedicatedTools.size() > 0 && listUniversalTools.size() > 0)
    {
        //分割线
        QLabel* pLabel = new QLabel();
        pLabel->setFixedSize(3, 20);
        pLabel->setStyleSheet("background-color: white; border: 1px solid black;");
        pLabel->setObjectName("SplitLine");
        ui->horizontalLayout->addWidget(pLabel);
    }
    for (int i = 0; i < listUniversalTools.size(); i++)
    {
        auto toolName = listUniversalTools[i];
        auto it = std::find_if(m_listUniversalTools.begin(), m_listUniversalTools.end(), [toolName](const ToolsInfo& item) {
            return item.sName == toolName;
            });
        if (it != m_listUniversalTools.end()) {

            QAction* pAction = new QAction();
            pAction->setIcon(QIcon(QString(kToolBarIconPath + m_listUniversalTools[i].sIconName)));
            pAction->setToolTip(m_listUniversalTools[i].sText);
            pAction->setText(m_listUniversalTools[i].sText);
            pAction->setObjectName(m_listUniversalTools[i].sName);
            connect(pAction, &QAction::triggered, this, [=]() {
                m_listUniversalTools[i].clickedCallback();
                });
            m_hashToolsAction.insert(m_listUniversalTools[i].sName, pAction);

            QToolButton* pBtn = new QToolButton(this);
            pBtn->setObjectName(m_listUniversalTools[i].sName);
            pBtn->setDefaultAction(pAction);
            btnIndex++;

            if (toolName == kNameTalk && m_pLiveWidget->IsAnyTalk()) {
                pAction->setIcon(QIcon(QString(kToolBarIconPath + "TalkStop.png")));
                pAction->setToolTip(tr("停止对讲"));
                pAction->setText(tr("停止对讲"));
            }
            if (toolName == kNameZoom && m_pLiveWidget->IsZoom()) {
                pAction->setToolTip(tr("关闭电子放大"));
                pAction->setText(tr("关闭电子放大"));
            }
            ui->horizontalLayout->addWidget(pBtn);
        }
        m_currentUniversalTools.push_back(toolName);
    }

    //创建more按钮
    QToolButton* pBtnMore = new QToolButton(this);
    pBtnMore->setObjectName("BtnMore");
    pBtnMore->setIcon(QIcon(":res/icon/icon/More.png"));
    pBtnMore->setVisible(false);
    ui->horizontalLayout->addWidget(pBtnMore);
    connect(pBtnMore, &QPushButton::clicked, this, &LiveBottomFloatingBar::on_pbMore_clicked);

    update();
}

void LiveBottomFloatingBar::on_StartRecord()
{
    auto pBtn = m_hashToolsAction.value(kNameRecord);
    if(pBtn)
    {
        pBtn->setToolTip(tr("停止录像"));
        pBtn->setIcon(QIcon(":/icon/icon/RecordStop.png"));
    }
}

void LiveBottomFloatingBar::on_StopRecord()
{
    auto pBtn = m_hashToolsAction.value(kNameRecord);
    if(pBtn)
    {
        pBtn->setToolTip(tr("开始录像"));
        pBtn->setIcon(QIcon(":/icon/icon/Record.png"));
    }
}

void LiveBottomFloatingBar::on_SetGas(bool isEnable)
{
    auto pBtn = m_hashToolsAction.value(kNameGas);
    if(pBtn)
    {
        if(isEnable){
            pBtn->setToolTip(tr("隐藏废气信息"));
        }else{
            pBtn->setToolTip(tr("显示废气信息"));
        }
    }
}

void LiveBottomFloatingBar::on_SetMask(bool isEnable)
{
    auto pBtn = m_hashToolsAction.value(kNameMask);
    if(pBtn)
    {
        if(isEnable){
            pBtn->setToolTip(tr("隐藏屏蔽区域"));
        }else{
            pBtn->setToolTip(tr("显示屏蔽区域"));
        }
    }
}

void LiveBottomFloatingBar::on_pbMore_clicked() {
    //MoreMenu moreMenu1(m_pLiveWidget);
    //moreMenu1.ShowWithCursor();
    m_pMoreMenu->clear();
    for (size_t i = 0; i < m_currentMenuTools.size(); i++)
    {
        auto toolName = m_currentMenuTools[i];
        auto pAction = m_hashToolsAction.value(toolName);
        if (pAction)
        {
            auto toolInfo = getToolsInfo(toolName);
            if (!toolInfo.sIconName.isEmpty())
            {
                pAction->setIcon(QIcon(QString(kToolBarMenuIconPath + toolInfo.sIconName)));
            }
            if (toolName == kNameInstantPlayBack)
            {
                m_pMoreMenu->addMenu(m_pMoreMenu->GetHistoryMenu());
            }
            else if (toolName == kNameStream)
            {
                m_pMoreMenu->addMenu(m_pMoreMenu->GetStreamMenu());
            }
            else
            {
                m_pMoreMenu->addAction(pAction);
            }
        }   
    }
    m_pMoreMenu->move(QCursor::pos());
    m_pMoreMenu->exec();
}

void LiveBottomFloatingBar::on_pbFEC_clicked()
{
}

void LiveBottomFloatingBar::on_pbFireFrame_clicked() {

}

void LiveBottomFloatingBar::on_pbGas_clicked()
{
}

void LiveBottomFloatingBar::on_pbMask_clicked()
{
}

void LiveBottomFloatingBar::on_pbWarnOutCtrl_clicked() {
}

void LiveBottomFloatingBar::on_pbSnap_clicked()
{
}

void LiveBottomFloatingBar::on_pbRecord_clicked()
{
}

void LiveBottomFloatingBar::on_pbTalk_clicked()
{

}

void LiveBottomFloatingBar::on_pbZoom_clicked()
{

}

void LiveBottomFloatingBar::on_pbInstantPlayBack_clicked()
{
    auto menu = m_pMoreMenu->GetHistoryMenu();
    menu->move(QCursor::pos());
    menu->exec();
}

void LiveBottomFloatingBar::on_pbStream_clicked()
{
    auto menu = m_pMoreMenu->GetStreamMenu();
    menu->move(QCursor::pos());
    menu->exec();
}

void LiveBottomFloatingBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int nAcceptShowNum =  (event->size().width() - 300) / 30;//被允许显示的图标最大个数,-5是为了按钮有足够的间隔，不必过分密集
    int nShowBtnIndex = 0;//当前有8个按钮显示在栏上，每个占用20宽

    QList<QString> tempMenuTools = m_currentDedicatedTools;
    tempMenuTools.append(m_currentUniversalTools);

    int nDedicatedToolsCount = m_currentDedicatedTools.size();
    int nUniversalToolsCount = m_currentUniversalTools.size();

    bool isSplitLineShow = false;
    if (nAcceptShowNum > nDedicatedToolsCount)
        isSplitLineShow = true;

    bool isMoreMenuShow = false;
    if (nAcceptShowNum < (nDedicatedToolsCount + nUniversalToolsCount))
        isMoreMenuShow = true;

    for (int i = 0; i < ui->horizontalLayout->count(); ++i)
    {
        QWidget* widget = ui->horizontalLayout->itemAt(i)->widget();
        if (widget)
        {
			if (widget->objectName() == "SplitLine")
			{
				widget->setVisible(isSplitLineShow);
				continue;
			}
			if (widget->objectName() == "BtnMore")
			{
				widget->setVisible(isMoreMenuShow);
				continue;
			}
            QToolButton* btn = qobject_cast<QToolButton*>(widget);
            if (btn && (nShowBtnIndex < nAcceptShowNum))
            {
                btn->setVisible(true);
                auto pAction = m_hashToolsAction.value(btn->objectName());
                auto toolInfo = getToolsInfo(btn->objectName());
                if(pAction && !toolInfo.sIconName.isEmpty())
                {
                    pAction->setIcon(QIcon(QString(kToolBarIconPath + toolInfo.sIconName)));
                }
                nShowBtnIndex++;
                continue;
            }
            widget->setVisible(false);
        }
    }

    tempMenuTools.erase(tempMenuTools.begin(), tempMenuTools.begin() + nShowBtnIndex);
    m_currentMenuTools = tempMenuTools;
}
