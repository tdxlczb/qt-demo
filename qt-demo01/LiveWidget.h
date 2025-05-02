#ifndef LIVEWIDGET_H
#define LIVEWIDGET_H

#include <QWidget>
#include <QResizeEvent>

class LiveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LiveWidget(QWidget *parent = nullptr);

    bool IsAnyTalk() { return false;}
    bool IsZoom() { return false;}
    void resizeEvent(QResizeEvent* event) override;
signals:
    void sig_SendJsonToClient(const QString& szMethod, QJsonObject& jsonObjRsp, const QString& szDesc, const int& iCode = 200);
};

#endif // LIVEWIDGET_H
