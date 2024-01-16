#include "DX11Debug.h"
#include <cstdio>

HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr,
    _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox)
{
    WCHAR strBufferFile[MAX_PATH];
    WCHAR strBufferLine[128];
    WCHAR strBufferError[300];
    WCHAR strBufferMsg[1024];
    WCHAR strBufferHR[40];
    WCHAR strBuffer[3000];

    //将报错码转换为字符串
    swprintf_s(strBufferLine, 128, L"%lu", dwLine);
    //向调试控制台输出文件和行信息
    if (strFile)
    {
        swprintf_s(strBuffer, 3000, L"%ls(%ls): ", strFile, strBufferLine);
        OutputDebugStringW(strBuffer);
    }
   // 向调试控制台输出附加信息
    size_t nMsgLen = (strMsg) ? wcsnlen_s(strMsg, 1024) : 0;
    if (nMsgLen > 0)
    {
        OutputDebugStringW(strMsg);
        OutputDebugStringW(L" ");
    }
    //读取与 HRESULT 代码相对应的错误信息
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        strBufferError, 256, nullptr);
    //删除错误信息中的换行符尾部
    WCHAR* errorStr = wcsrchr(strBufferError, L'\r');
    if (errorStr)
    {
        errorStr[0] = L'\0';   
    }
    //将 HRESULT 代码附加到错误信息中
    swprintf_s(strBufferHR, 40, L" (0x%0.8x)", hr);
    wcscat_s(strBufferError, strBufferHR);
    //向调试控制台输出格式化的错误信息
    swprintf_s(strBuffer, 3000, L"错误码含义：%ls", strBufferError);
    OutputDebugStringW(strBuffer);
    //向调试控制台输出换行符
    OutputDebugStringW(L"\n");
    //显示包含调试信息的消息框
    if (bPopMsgBox)
    {
        wcscpy_s(strBufferFile, MAX_PATH, L"");
        if (strFile)
            wcscpy_s(strBufferFile, MAX_PATH, strFile);

        wcscpy_s(strBufferMsg, 1024, L"");
        if (nMsgLen > 0)
            swprintf_s(strBufferMsg, 1024, L"当前调用：%ls\n", strMsg);

        swprintf_s(strBuffer, 3000, L"文件名：%ls\n行号：%ls\n错误码含义：%ls\n%ls您需要调试当前应用程序吗？",
            strBufferFile, strBufferLine, strBufferError, strBufferMsg);

        int nResult = MessageBoxW(GetForegroundWindow(), strBuffer, L"错误", MB_YESNO | MB_ICONERROR);
        if (nResult == IDYES)
            DebugBreak();
    }

    return hr;
}