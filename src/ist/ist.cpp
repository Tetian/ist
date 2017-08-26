#include "ist.h"
#include <Windows.h>
#include <list>
#include "util.h"
#include "shell.h"
bool __stdcall ISTInstallInit(int is_type, const wchar_t* skin_dir, const wchar_t* skin_name) {
    return skin_dir&&skin_name ? shell::InitShell(is_type, skin_dir, skin_name) : false;
}

void __stdcall ISTSetInstallDir(const wchar_t* install_dir) {
    shell::SetInstallDir(install_dir ? install_dir : L"");
}

void __stdcall ISTSetCustomData(const wchar_t* data) {
    shell::SetCustomData(data ? data : L"");
}

void __stdcall ISTShowShell() {
    shell::ShowWin();
}

int __stdcall ISTWaitUserAction() {
    return shell::WaitAction();
}

void __stdcall ISTGetInstallPath(wchar_t* ret, int len) {
    if (!ret)return;
    std::wstring ip = shell::GetInstallPath();
    wcscpy_s(ret, len, ip.c_str());
}

void __stdcall ISTSetProgress(int now, int max, const wchar_t* msg) {
    shell::SetProgress(now, max, msg ? msg : L"");
}

void __stdcall ISTSetFileName(const wchar_t* msg) {
    shell::SetFileName(msg ? msg : L"");
}

void __stdcall ISTSetStatus(const wchar_t* msg) {
    shell::SetStatus(msg ? msg : L"");
}

int __stdcall ISTWaitCompleted() {
    return shell::ChangeCompleteAndWait();
}

int __stdcall ISTShowMessage(const wchar_t* msg, const wchar_t* caption, int type) {
    return shell::ShowMessage(msg ? msg : L"", caption ? caption : L"", type);
}

int __stdcall ISTProcessExisted(int type, const wchar_t* param1, int should_kill, const wchar_t* param2) {
    return util::ProcessExisted(type, param1 ? param1 : L"", should_kill, param2 ? param2 : L"");
}

void __stdcall ISTSetProcessCheck(int type, const wchar_t* dir, const wchar_t* ignore, wchar_t* exe_list) {
    shell::SetProcessCheck(type, dir ? dir : L"", ignore ? ignore : L"", exe_list ? exe_list : L"");
}