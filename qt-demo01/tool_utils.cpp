#include "tool_utils.h"
#include <QList>
#include <QRegularExpression>
#include <QDebug>
#include <Windows.h>

namespace  {
struct StWindowTitleInfo {      //窗口标题信息
    HWND hwnd;              //窗口句柄
    QString title;          //窗口标题
};

}

ToolUtils& ToolUtils::GetInstance()
{
    static ToolUtils instance;
    return instance;
}

BOOL CALLBACK ToolUtils::enumWindowsProcByTitle(HWND hwnd, LPARAM lParam) {
    // 获取窗口标题
    wchar_t windowTitle[256];
    GetWindowText(hwnd, windowTitle, sizeof(windowTitle)/sizeof(wchar_t));

    // 如果窗口标题不为空，将其保存
    if (wcslen(windowTitle) > 0) {
        QString title = QString::fromWCharArray(windowTitle);
        QList<StWindowTitleInfo>* windowList = reinterpret_cast<QList<StWindowTitleInfo>*>(lParam);
        windowList->append({ hwnd, title });
    }

    return TRUE; // 继续枚举其他窗口
}

HWND ToolUtils::FindWindowByRegex(const QString &pattern)
{
    // 创建用于保存所有窗口信息的列表
    QList<StWindowTitleInfo> windowList;

    // 枚举所有窗口
    EnumWindows(enumWindowsProcByTitle, reinterpret_cast<LPARAM>(&windowList));

    // 使用正则表达式查找符合条件的窗口
    QRegularExpression regex(pattern);
    for (const StWindowTitleInfo &window : windowList) {
        if (regex.match(window.title).hasMatch()) {
            //qDebug() << window.title;
            qInfo() << "匹配到窗口:" << window.title;
            return window.hwnd; // 返回第一个匹配的窗口句柄
        }
    }

}
