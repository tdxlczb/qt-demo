#ifndef MoreMenu_H
#define MoreMenu_H

#include <QWidget>
#include "BaseMenu.h"
class LiveWidget;


class MoreMenu : public BaseMenu
{
    Q_OBJECT

public:
    explicit MoreMenu(LiveWidget *parent);

    void CreateHistoryMenu();
    void CreateStreamMenu();
    QMenu* GetHistoryMenu();
    QMenu* GetStreamMenu();
private:
    //多级菜单
    QMenu *m_pMenuHistory;       //切换至即时回放
    QMenu *m_pMenuStream;        //码流

    QActionGroup *m_pActGroupStream; //码流组

private slots:
    //请求前端开始语音对讲:指定语音通道号
    void on_startVoiceIntercom(int iVoiceChannel);
    //请求前端开始IP通道对讲
    void on_startIpVoiceIntercom();
    //请求前端停止对讲
    void on_stopVoiceIntercom();
    //请求前端开始电子放大
    void on_startZoom();
    //请求前端停止电子放大
    void on_stopZoom();
    //请求前端以指定秒数进行即时回放
    void on_instantPlayBack(int iSec);
    //请求前端切换到指定码流,当自动码流为true时不向前端发送码流请求，而是向程序内发送自动码流的信号以尝试根据具体情况切换到具体码流
    void on_switchStream(bool isStreamAuto,int iStreamIndex);
};

#endif // MoreMenu_H
