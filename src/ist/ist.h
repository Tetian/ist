#pragma once
#include "const.h"
//安装服务初始化 使用DUILIB UILIB_ZIP初始皮肤
//is_type 1:安装 2:卸载
//skin_dir 皮肤路径
//skin_name 皮肤文件
//返回 true设置成功
DLL_EXPORT bool __stdcall ISTInstallInit(int is_type, const wchar_t* skin_dir, const wchar_t* skin_name);

//设置初始界面安装路径
//install_dir 路径字串
DLL_EXPORT void __stdcall ISTSetInstallDir(wchar_t* install_dir);

//设置自定义数据
//data 自定义数据
DLL_EXPORT void __stdcall ISTSetCustomData(const wchar_t* data);

//设置自定义数据
//data 自定义数据
DLL_EXPORT void __stdcall ISTShowShell();

//等待用户选择是否安装
//返回 安装时:-1点击了关闭按钮退出 1点击了"开始安装""
//卸载时: -1点击了关闭 3点击了"开始卸载"
DLL_EXPORT int __stdcall ISTWaitUserAction();

//获取安装用户选择的路径
//ret 内容指针
//len 内容长度
DLL_EXPORT void __stdcall ISTGetInstallPath(const wchar_t* ret, int len);

//设置当前安装的进度
//now 当前安装的进度值
//max 安装进度最大值
//msg 信息
DLL_EXPORT void __stdcall ISTSetProgress(int now, int max, const wchar_t* msg);

//设置安装文件信息
//msg 文件信息
DLL_EXPORT void __stdcall ISTSetFileName(const wchar_t* msg);

//设置安装状态信息
//msg 状态信息
DLL_EXPORT void __stdcall ISTSetStatus(const wchar_t* msg);

//等待用户点击完成
//返回 安装时:-1点击关闭按钮退出 2点击"立即体验"
//卸载时:-1点击关闭按钮 4卸载点击"卸载完毕"
DLL_EXPORT int __stdcall ISTWaitCompleted();

//显示模态提示框 未初始化皮肤时使用系统MessageBox
//msg 消息
//caption 标题
//type 类型 0:MB_OK 其他:MB_OKCANCEL
//返回 IDOK(1) IDCANCEL(2)
DLL_EXPORT int __stdcall ISTShowMessage(const wchar_t* msg, const wchar_t* caption, int type);

//----------------------其他辅助接口----------------------

//检查进程是否存在,并可设置尝试kill
//type 处理类型
//0:单进程处理
//1:进程名组处理
//2:文件夹处理,遍历文件夹内所有exe是否是正在运行的进程,会通过获取进程的完整路径进行路径比较
//如: D:\dir\a.exe D:\dir\b.exe D:\dir\c.exe D:\a.exe 运行的进程有a.exe b.exe设定路径为D:\dir 则只会对dir文件中的a.exe b.exe进行处理
//3:参数被包含进程处理,进程的参数是否包含了某些特定字符
//如: apache有多个进程 httpd.exe无参数和 参数包含-d及路径 需要关闭-d的进程
//4:参数不被包含进程处理,进程的参数不包含特定字符

//param1 参数1 根据type确定:
//type=0 单进程名"a.exe"
//type=1 进程名组"a.exe,b.exe"
//type=2 文件夹路径
//type=3或4 单进程名

//should_kill 进程运行是否尝试结束 1:尝试结束 0:不结束

//param2 参数2 根据type确定:
//type=0或1 保留项
//type=2 忽略目标 "ignore1.exe,ignore2.exe"
//type=3或4 被包含或不被包含命令行

//返回 0不存在或存在被kill掉 1存在 2存在但kill不掉:根据should_kill设置 1:返回0或2 0:返回0或1
DLL_EXPORT int __stdcall ISTProcessExisted(int type, const wchar_t* param1, int should_kill, const wchar_t* param2);

//常规进程检查:使用文件夹+进程组方式.
//文件夹适用情况:程序后期更新后可能会有新的运行进程exe放入安装目录 会自动排除掉卸载程序{#IST_UNINSTALL_EXE}
//进程组适用情况:可能有其他不在安装目录中的关联运行进程;不受现在安装包控制的老版本运行进程 可和文件夹方式协同重复检查
//因进程名极可能重复,所以进程组参数设定极可能影响安装的运行进程即可
//type 1询问方式 2自动关闭 dir检查的文件夹;ignore忽略的exe组;exe_list进程组
DLL_EXPORT void __stdcall ISTSetProcessCheck(int type, const wchar_t* dir, const wchar_t* ignore, wchar_t* exe_list);
