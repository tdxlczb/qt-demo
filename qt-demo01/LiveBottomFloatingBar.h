#ifndef LIVEBOTTOMFLOATINGBAR_H
#define LIVEBOTTOMFLOATINGBAR_H

#include <functional>
#include <QWidget>
#include <QList>
#include <QHash>
#include <QAction>
//#include "Common/commondef.h"
class LiveWidget;
class MoreMenu;
namespace Ui {
class LiveBottomFloatingBar;
}

struct StWebInfo
{

};

struct ToolsInfo
{
    QString sName;
    QString sIconName;
    QString sText;
    std::function<void()> clickedCallback;
};

class LiveBottomFloatingBar : public QWidget
{
    Q_OBJECT

public:
    explicit LiveBottomFloatingBar(LiveWidget *parent);
    ~LiveBottomFloatingBar();

private:
    void initControl();
    ToolsInfo getToolsInfo(const QString& toolName);

private slots:
    //应用来自MediaWidget的全局配置变更
    void on_UpdateConfig(const StWebInfo& stWebInfo);
    //被通知已进入开始录制的状态
    void on_StartRecord();
    //被通知已退出录制的状态
    void on_StopRecord();
    //被通知更新废气状态
    void on_SetGas(bool isEnable);
    //被通知更新屏蔽区域状态
    void on_SetMask(bool isEnable);

    //弹出更多菜单
    void on_pbMore_clicked();

    void on_pbFEC_clicked();
    //弹出火点菜单
    void on_pbFireFrame_clicked();
    void on_pbGas_clicked();
    void on_pbMask_clicked();
    //未实现：因无报警输入输出设备
    void on_pbWarnOutCtrl_clicked();

    void on_pbSnap_clicked();
    void on_pbRecord_clicked();
    void on_pbTalk_clicked();
    void on_pbZoom_clicked();
    void on_pbInstantPlayBack_clicked();
    void on_pbStream_clicked();

    void resizeEvent(QResizeEvent *event) override;
private:
    Ui::LiveBottomFloatingBar *ui;
    LiveWidget * m_pLiveWidget;
    QList<ToolsInfo> m_listUniversalTools;
    QList<ToolsInfo> m_listDedicatedTools;
    QHash<QString, QAction*> m_hashToolsAction;
    QList<QString> m_currentUniversalTools;
    QList<QString> m_currentDedicatedTools;
    QList<QString> m_currentMenuTools;

    //多级菜单
    MoreMenu* m_pMoreMenu = nullptr;
};

#endif // LIVEBOTTOMFLOATINGBAR_H
