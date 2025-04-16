#ifndef TOOLUTILS_H
#define TOOLUTILS_H

#include <Windows.h>
#include <QString>

class ToolUtils
{
public:
    static ToolUtils&GetInstance();

    ///
    /// \brief 查找窗口句柄
    /// \param digitLen 数字位数
    /// \return
    ///
    HWND FindWindowByRegex(const QString &pattern);

    ///
    /// \brief 获取随机数字
    /// \param digitLen 数字位数
    /// \return
    ///
    uint32_t GetRandomNumber(uint32_t digitLen = 8);

private:
    static BOOL CALLBACK enumWindowsProcByTitle(HWND hwnd, LPARAM lParam);
private:
    ToolUtils() {};
    ~ToolUtils() {};
    ToolUtils(const ToolUtils&)= delete;
    ToolUtils &operator=(const ToolUtils&) = delete;
};

#endif // TOOLUTILS_H
