#include "main_window.h"

#include <QApplication>

#include "log.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonParseError>
#include <QDataStream>

const QString data = "{\"method\":\"media.setWindowInitConfig\",\"info\":{\"PTZDisplayUserInfo\":\"1\",\"PTZReleaseTime\":\"0\",\"adaptivePictureLayout\":\"1\",\"adaptiveStream\":\"1\",\"addWatermark\":\"1\",\"authenticationType\":\"1\",\"autoRecord\":\"1\",\"cannotCoverSavingPool\":\"\",\"captureMode\":\"0\",\"cascadeEncodingType\":\"1\",\"coveredSavingPool\":\"\",\"deviceTiming\":\"1\",\"eventRetentionDuration\":\"1\",\"fieldDesensitization\":\"0\",\"fileStoragePath\":\"\",\"filterOfflinePoints\":\"1\",\"gpuHardDecode\":\"0\",\"imageMonitor\":\"0\",\"imageMonitorTime\":\"30\",\"limitedTimePreview\":\"0\",\"limitedTimePreviewDuration\":\"30\",\"loginType\":\"1,2\",\"mainStreamAdaptation\":\"1\",\"memoryPreviewWindow\":\"1\",\"mobileUserBinding\":\"0\",\"oneKeyWallPost\":\"1\",\"onlineStatus\":\"1\",\"osdTemplate\":\"{\\\"channelName\\\":{\\\"enabled\\\":true,\\\"left\\\":0,\\\"right\\\":0,\\\"top\\\":0,\\\"bottom\\\":0},\\\"dateTime\\\":{\\\"enabled\\\":true,\\\"hourOSDType\\\":0,\\\"left\\\":0,\\\"right\\\":0,\\\"top\\\":0,\\\"bottom\\\":0,\\\"osdType\\\":0},\\\"displayWeek\\\":false,\\\"osdAttribute\\\":1}\",\"pageWatermark\":\"1\",\"passwordExpirationDate\":\"1\",\"passwordValidityPeriod\":\"0\",\"playbackStreamFetch\":\"\",\"postDecodeBuffer\":\"6\",\"preDecodeBuffer\":\"1024\",\"previewPostDecodeBuffer\":\"6\",\"previewPreDecodeBuffer\":\"1024\",\"previewStreamFetch\":\"\",\"previewToolbar\":\"\",\"ptzMode\":\"1\",\"reconnectTimeGap\":\"10000\",\"retainPicture\":\"1\",\"securityAreaScenario\":\"1\",\"streamFetchReconnectCount\":\"3\",\"streamSwitchCount\":\"4\",\"timingCycle\":\"1\",\"userOnlineLimit\":\"200\",\"videoEditConfig\":\"0\",\"wCaptureImageFormat\":\"1\",\"wIntelligentRuleDisplay\":\"0\",\"wPlaybackToolbar\":\"\",\"watermarkContent\":\"\",\"watermarkTextColor\":\"1\",\"watermarkTransparency\":\"10\",\"captureImageFormat\":\"bmp\",\"recordPath\":\"\",\"snapShotPath\":\"\",\"decodeType\":2,\"snapType\":1,\"clientType\":1},\"id\":0}";


void JsonTest()
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);

    if (!jsonDoc.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            if (jsonObj.contains("method")) {
                QJsonValue nameValue = jsonObj.value("method");
                QVariant tn = nameValue.toVariant();
                int a = tn.toInt();
                if (nameValue.isString()) {

                    QString name = nameValue.toString();
                }
            }
        }
    }


}


#include "main_application.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
//    MainApplication a(argc, argv);

    qlog::InstallLog(a.applicationDirPath(), a.applicationName());

    JsonTest();
    //以后就可以像下面这样直接打日志到文件中，而且日志也会包含时间信息
    qDebug("This is a debug message at thisisqt.com");
    qWarning("This is a warning message at thisisqt.com");
    qCritical("This is a critical message at thisisqt.com");
    //qFatal("This is a fatal message at thisisqt.com");

    MainWindow w;
    w.show();
    return a.exec();

//    QWidget widget;
//    widget.setFixedSize(0,0);
//    widget.show();
//    return a.exec();
}
