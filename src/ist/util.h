#pragma once
#include <Windows.h>
#include <string>
#include <sstream>

namespace util {
    //获取实例句柄
	HMODULE GetModuleFromAddr(PVOID p = NULL);

	//窗口置顶设置
	void TopMost(bool is_top, HWND hwnd);

	//获取进程窗口句柄
	HWND GetProcessHwnd(DWORD pid);

	//文件是否存在
	bool FileExsit(const std::wstring& file);

	//文件夹是否存在
	bool DirExsit(const std::wstring& dir);

	//转为宽字适合数字
	template <class T>
	std::wstring ToStdWString(T data) {
		std::wstringstream wstream;
		wstream << data;
		std::wstring result(wstream.str());
		return result;
	}

	//隐藏当前进程窗口
	bool HideThisProcessWindow();

	//获取路径末尾文件名
	std::wstring GetPathFileName(const std::wstring& path);

    //进程名检测
	int ProcessExisted(const std::wstring& pro_name, int should_kill);

    //进程名组检查
	int ProcessExistedList(const std::wstring& pro_name_s, int should_kill);

    //进程文件及路径检查
	int ProcessExistedDir(const std::wstring& dir, int should_kill, const std::wstring& p_ignore_s);

    //进程名+进程命令行检查
	int ProcessExistedByCL(const std::wstring& pro_name, int should_kill,const std::wstring cl);

    //进程名+无命令行检查
    //如appche web服务容器 多进程 其中无特定命令行参数的为主进程
	int ProcessExistedNoCL(const std::wstring& pro_name, int should_kill, const std::wstring cl);

    //进程检查总入口
	int ProcessExisted(int type, const std::wstring& param1, int should_kill, const std::wstring& param2);

    //验证路径磁盘
    bool IsValidDisk(const std::wstring& path);

    //获取 %APPDATA%\..\Local
    std::wstring GetLocalAppDataPath();

    //判断是否是XP系统
    bool IsWinXP();

    //是否是XPSP3以上系统
    bool IsWinXPSP3orGreater();
}
