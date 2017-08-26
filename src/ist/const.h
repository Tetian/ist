#pragma once
#ifndef DLL_EXPORT
#define DLL_EXPORT extern "C" __declspec(dllexport)
#endif // !EXTERN_DLL_EXPORT

namespace msg{
    const wchar_t C003[] = L"应用程序初始化失败";
    const wchar_t C004[] = L"初始化错误：安装设定的路径获取错误";
    const wchar_t C005[] = L"检查到不支持的操作系统\n需要Windows XP SP3以上操作系统。";
    const wchar_t C006[] = L"安装路径不能为磁盘根目录";
    const wchar_t C007[] = L"无法结束关联程序，请手动关闭后再安装。";
    const wchar_t C008[] = L"检查到有关联的程序正在运行，继续操作需要将之关闭\n确认继续操作？";
    const wchar_t C010[] = L"获取安装路径错误";
}