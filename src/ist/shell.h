#pragma once

#include <string>
namespace shell {
    //初始化
    bool InitShell(int is_type, const std::wstring& tmp_dir, const std::wstring& skin_name);

    //设置路径
    void SetInstallDir(const std::wstring& install_dir);

    //设置自定义数据
    void SetCustomData(const std::wstring& data);

    //等待操作
    int WaitAction();

    //设置进度
    void SetProgress(int now, int max, const std::wstring& msg);

    //设置安装文件名
    void SetFileName(const std::wstring& msg);

    //设置状态
    void SetStatus(const std::wstring& msg);

    //切至完成并等待操作
    int ChangeCompleteAndWait();

    //获取安装路径
    std::wstring GetInstallPath();

    //设定
    int ShowMessage(const std::wstring& msg, const std::wstring& caption, int type);

    //设定进程检查方式
    void SetProcessCheck(int pc_type, const std::wstring& dir, const std::wstring& ignore, const std::wstring& exe_list);

    //IS类型
    int GetISType();

    //DLL加载标识
    bool DllLoading();

    //完全释放
    void AllDetach();
    
    //显示窗口
    void ShowWin();
}