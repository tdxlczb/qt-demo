#ifndef TOOLUTILS_H
#define TOOLUTILS_H

#include <Windows.h>
#include <QString>

class ToolUtils
{
public:
    static ToolUtils&GetInstance();
    HWND FindWindowByRegex(const QString &pattern);
private:
    static BOOL CALLBACK enumWindowsProcByTitle(HWND hwnd, LPARAM lParam);
private:
    ToolUtils() {};
    ~ToolUtils() {};
    ToolUtils(const ToolUtils&)= delete;
    ToolUtils &operator=(const ToolUtils&) = delete;
};

#endif // TOOLUTILS_H
