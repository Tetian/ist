#include "shell.h"
//引入duilib
#include "duilib_ori\UIlib.h"

#include "ui_message_box.h"
#include "util.h"
#include "const.h"

#include <memory>
#include <shlwapi.h>
#include <process.h>
#include <shlobj.h>
#include <thread>
#include <mutex>

namespace shell {
    using namespace DuiLib;

    namespace {
        //设置窗口到完成状态
#define SHELL_WIN_TO_COMPLETE WM_USER+1

        typedef enum {
            //安装初始
            kLSFirst,
            //安装中
            kLSSecond,
            //安装完毕
            kLSThird,
            //卸载初始
            kLSFourth,
            //卸载中
            kLSFifth,
            //卸载完毕
            kLSSixth
        } LayoutStatus;


        //安装类型 1安装 2卸载
        int is_type_ = 0;

        class CShellWin;

        //主窗口
        std::shared_ptr<CShellWin> shell_win_;

        //主窗口句柄 在WM_CLOSE时会设为NULL
        HWND shell_win_hwnd_ = NULL;

        //用户等待操作标识
        int user_wait_action_ = 0;

        //std::thread线程锁
        std::mutex mutex_;

        //安装目录
        std::wstring install_dir_;

        //自定义数据
        std::wstring custom_data_;

        //安装目录尾
        std::wstring install_dir_tail_;

        //进程检查类型
        int pc_type_ = -1;

        //进程检查文件夹
        std::wstring pc_dir_;

        //进程检查忽略文件
        std::wstring pc_ignore_;

        //进程检查进程组
        std::wstring pc_exe_list_;

        //自定义Msgbox标题
        std::wstring msg_box_title_ = L"提示";

        //dll加载标识
        bool dll_loading_ = true;

        //窗口类
        class CShellWin :public WindowImplBase
        {
        public:
            CShellWin() {}
            ~CShellWin() {}

            //设置自定义数据
            void SetCustomData() {
                if (lb_logo_) {
                    lb_logo_->SetText(custom_data_.c_str());
                }
            }

            CDuiString GetSkinFolder() override {
                return _T("");
            }

            LPCTSTR GetWindowClassName(void) const {
                return _T("ISTShellWin");
            }

            CDuiString GetSkinFile() override {
                return _T("shell_win.xml");
            }

            void InitWindow() override {
                shell_win_hwnd_ = m_hWnd;
                this->CenterWindow();
                //取消置顶
                util::TopMost(false, m_hWnd);
                SetWindowText(m_hWnd, L"安装");
                btn_close_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_close")));
                lb_logo_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lb_logo")));
                layout_first_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_first")));
                layout_second_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_second")));
                layout_third_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_third")));
                layout_fourth_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_fourth")));
                layout_fifth_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_fifth")));
                layout_sixth_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_sixth")));

                edit_path_ = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_path")));
                btn_dir_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_dir")));

                layout_progress_ = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("layout_progress")));
                lb_file_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lb_file")));
                p_install_ = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("p_install")));
                lb_install_per_ = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lb_install_per")));
                ring_install_ = static_cast<CGifAnimUI*>(m_PaintManager.FindControl(_T("ring_install")));
                install_status_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("install_status")));

                btn_install_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_install")));
                btn_complete_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_complete")));

                btn_uninstall_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_uninstall")));

                ring_uninstall_ = static_cast<CGifAnimUI*>(m_PaintManager.FindControl(_T("ring_uninstall")));
                uninstall_status_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("uninstall_status")));

                btn_uninstall_complete_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_uninstall_complete")));
            }

            LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
                switch (uMsg) {
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP: {
                    //屏蔽Atl+F4
                    unsigned int key_code = wParam;
                    bool out_key = ((key_code == VK_F4) &&
                        (GetKeyState(VK_MENU) < 0));
                    if (out_key) return TRUE;
                }
                case WM_NCLBUTTONDBLCLK: {
                    //双击标题栏事件无效化
                    return TRUE;
                }
                case WM_CLOSE: {
                    shell_win_hwnd_ = NULL;
                    break;
                }
                case SHELL_WIN_TO_COMPLETE: {
                    this->ChangeToComplete();
                    return TRUE;
                }
                default:
                    break;
                }

                return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
            }


            void CustomShow() {
                shell_win_->SetCustomData();
                if (1 == is_type_) {
                    SetInstallDir();
                    SetLayoutStatus(kLSFirst);
                }
                else if (2 == is_type_) {
                    SetLayoutStatus(kLSFourth);
                }
                this->ShowModal();
            }

            void SetProgress(int now, int max, const std::wstring& status_msg) {
                if (p_install_) {
                    p_install_->SetMaxValue(max);
                    p_install_->SetValue(now);
                    p_install_->SetText(status_msg.c_str());
                }
                if (lb_install_per_) {
                    lb_install_per_->SetText(std::wstring(util::ToStdWString((int)(100.0f*now / max)) + L"%").c_str());
                }
            }

            void SetFileName(const std::wstring& status_msg) {
                if (lb_file_)lb_file_->SetText(status_msg.c_str());
            }

            void ChangeToComplete() {
                if (1 == is_type_) {
                    SetLayoutStatus(kLSThird);
                }
                else if (2 == is_type_) {
                    SetLayoutStatus(kLSSixth);
                }
            }

            std::wstring GetInstallPath() {
                if (nullptr == edit_path_ || edit_path_->GetText().IsEmpty()) {
                    this->ShowMessage(msg::C010, msg_box_title_, MB_OK);
                    return L"";
                }
                return std::wstring(edit_path_->GetText());
            }

            void SetInstallDir() {
                if (edit_path_) {
                    edit_path_->SetText(install_dir_.c_str());
                }
            }

            int ShowMessage(const std::wstring& msg, const std::wstring& caption, int type) {
                return ui::MessageBoxU(this->m_hWnd, msg, caption, type);
            }

            void SetStatus(const std::wstring& msg) {
                if (1 == is_type_) {
                    if (install_status_) {
                        install_status_->SetText(msg.c_str());
                    }
                }
                else if (2 == is_type_) {
                    if (uninstall_status_) {
                        uninstall_status_->SetText(msg.c_str());
                    }
                }
            }

            void SetLayoutStatus(int status) {
                //安装中和卸载中不能关闭
                btn_close_->SetEnabled(kLSSecond == status || kLSFifth == status ? false : true);

                layout_first_->SetVisible(kLSFirst == status);
                layout_second_->SetVisible(kLSSecond == status);
                layout_third_->SetVisible(kLSThird == status);
                layout_fourth_->SetVisible(kLSFourth == status);
                layout_fifth_->SetVisible(kLSFifth == status);
                layout_sixth_->SetVisible(kLSSixth == status);
            }
        private:
            CButtonUI* btn_close_;
            CLabelUI* lb_logo_;
            CVerticalLayoutUI* layout_first_;
            CVerticalLayoutUI* layout_second_;
            CVerticalLayoutUI* layout_third_;
            CVerticalLayoutUI* layout_fourth_;
            CVerticalLayoutUI* layout_fifth_;
            CVerticalLayoutUI* layout_sixth_;

            CEditUI* edit_path_;
            CButtonUI* btn_dir_;

            CVerticalLayoutUI* layout_progress_;
            CLabelUI* lb_file_;
            CProgressUI* p_install_;
            CLabelUI* lb_install_per_;
            CGifAnimUI* ring_install_;
            CButtonUI* install_status_;

            CButtonUI* btn_install_;
            CButtonUI* btn_complete_;

            CButtonUI* btn_uninstall_;

            CGifAnimUI* ring_uninstall_;
            CButtonUI* uninstall_status_;

            CButtonUI* btn_uninstall_complete_;

            //选择安装路径
            void SelectInstallDir() {
                TCHAR szBuffer[MAX_PATH] = { 0 };
                BROWSEINFO bi;
                ZeroMemory(&bi, sizeof(BROWSEINFO));
                bi.hwndOwner = GetForegroundWindow();
                bi.pszDisplayName = szBuffer;
                bi.lpszTitle = _T("选择安装路径");
                bi.ulFlags = BIF_RETURNFSANCESTORS;
                LPITEMIDLIST idl = SHBrowseForFolder(&bi);
                if (idl&&edit_path_) {
                    SHGetPathFromIDList(idl, szBuffer);
                    std::wstring e_path(szBuffer);
                    wchar_t os_path[MAX_PATH] = { 0 };
                    GetSystemDirectory(os_path, MAX_PATH);
                    if (!util::IsWinXP() && 0 == e_path.find(os_path[0])) {
                        //非XP系统选择了系统盘
                        e_path = util::GetLocalAppDataPath();
                    }
                    if (!install_dir_tail_.empty() && e_path.rfind(install_dir_tail_) == std::string::npos) {
                        //自动增加尾路径
                        e_path = e_path +
                            ((e_path.back() == L'\\' || e_path.back() == L'/') ? L"" : L"\\") +
                            install_dir_tail_;
                    }
                    if (TRUE == PathStripToRoot(szBuffer)) {
                        if (e_path == std::wstring(szBuffer)) {
                            this->ShowMessage(msg::C006, msg_box_title_, MB_OK);
                            return;
                        }
                    }
                    edit_path_->SetText(e_path.c_str());
                }
            }

            //进程检查
            bool ProcecssCheck() {
                if (2 == pc_type_) {
                    if (0 != util::ProcessExisted(2, pc_dir_, 1, pc_ignore_) ||
                        0 != util::ProcessExisted(1, pc_exe_list_, 1, L"")) {
                        this->ShowMessage(msg::C007, msg_box_title_, MB_OK);
                        return false;
                    }
                }
                else if (1 == pc_type_) {
                    if (0 != util::ProcessExisted(2, pc_dir_, 0, pc_ignore_) ||
                        0 != util::ProcessExisted(1, pc_exe_list_, 0, L"")) {
                        if (IDOK == this->ShowMessage(msg::C008, msg_box_title_, MB_OKCANCEL)) {
                            if (0 != util::ProcessExisted(2, pc_dir_, 1, pc_ignore_) ||
                                0 != util::ProcessExisted(1, pc_exe_list_, 1, L"")) {
                                this->ShowMessage(msg::C007, msg_box_title_, MB_OK);
                                return false;
                            }
                        }
                        else return false;
                    }
                }
                return true;
            }

            //预检查
            bool PreCheck(int type) {
                bool all_right = false;
                if (1 == type) {
                    std::wstring install_path = std::wstring(edit_path_->GetText());
                    all_right = util::IsValidDisk(install_path);
                    if (!all_right) {
                        this->ShowMessage(L"安装路径" + install_path + L"无效\n请选择有效路径", msg_box_title_, MB_OK);
                        return all_right;
                    }
                }
                all_right = ProcecssCheck();
                return all_right;
            }

            DUI_DECLARE_MESSAGE_MAP()
                virtual void OnClick(TNotifyUI& msg) {
                CDuiString c_name = msg.pSender->GetName();
                if (0 == wcscmp(c_name, btn_close_->GetName())) {
                    user_wait_action_ = -1;
                    this->Close();
                    return;
                }
                else if (0 == wcscmp(c_name, btn_install_->GetName())) {
                    if (!PreCheck(1)) return;
                    SetLayoutStatus(kLSSecond);
                    user_wait_action_ = 1;
                    return;
                }
                else if (0 == wcscmp(c_name, btn_complete_->GetName())) {
                    user_wait_action_ = 2;
                    this->Close();
                    return;
                }
                else if (0 == wcscmp(c_name, btn_dir_->GetName())) {
                    this->SelectInstallDir();
                    return;
                }
                else if (0 == wcscmp(c_name, btn_uninstall_->GetName())) {
                    if (!PreCheck(2)) return;
                    SetLayoutStatus(kLSFifth);
                    user_wait_action_ = 3;
                    return;
                }
                else if (0 == wcscmp(c_name, btn_uninstall_complete_->GetName())) {
                    user_wait_action_ = 4;
                    this->Close();
                    return;
                }
            }
        }; //calss

        DUI_BEGIN_MESSAGE_MAP(CShellWin, CNotifyPump)
            DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
            DUI_END_MESSAGE_MAP()


            //Dll释放处理
            HMODULE g_hdll_ = NULL;

        //循环检查标志自动释放dll
        DWORD WINAPI UnloadProc(PVOID param) {
            while (dll_loading_) {
                Sleep(100);
            }
            FreeLibraryAndExitThread(g_hdll_, 0);
            return 0;
        }
        //设置dll管理
        void DllManage() {
            if (is_type_ == 2) {
                g_hdll_ = DuiLib::CPaintManagerUI::GetInstance();
                HANDLE hThread = CreateThread(NULL, 0, UnloadProc, NULL, 0, NULL);
                CloseHandle(hThread);
            }
        }
    }//namespace


    bool InitShell(int is_type, const std::wstring& tmp_dir, const std::wstring& skin_name) {
        is_type_ = is_type;
        SetDllDirectory(tmp_dir.c_str());

        if (!util::IsWinXPSP3orGreater()) {
            //需要XPSP3系统以上
            MessageBox(NULL, msg::C005, L"安装错误", MB_OK);
            return false;
        }
        //隐藏进程窗口
        bool ret = util::HideThisProcessWindow();
        if (ret) {
            if (util::DirExsit(tmp_dir)) {
                //初始化皮肤资源
                CPaintManagerUI::SetResourcePath(tmp_dir.c_str());
                if (!skin_name.empty()) {
                    //缓存zip 释放dll用
                    CPaintManagerUI::SetResourceZip(skin_name.c_str(), true);
                }
                //设置messagebox资源
                ui::SetMessageBoxRes(L"messagebox.xml");
                DllManage();
            }
            else {
                ret = false;
            }
        }
        return ret;
    }

    void SetInstallDir(const std::wstring& install_dir) {
        install_dir_ = install_dir;
        install_dir_tail_ = util::GetPathFileName(install_dir_);
        if (shell_win_) {
            shell_win_->SetInstallDir();
        }
    }

    void SetCustomData(const std::wstring& data) {
        custom_data_ = data;
        if (shell_win_) {
            shell_win_->SetCustomData();
        }
    }

    int WaitAction() {
        //循环等待操作
        while (0 == user_wait_action_) {
            Sleep(500);
        }
        int tmp_action = user_wait_action_;
        if (-1 == user_wait_action_ || 2 == user_wait_action_ || 4 == user_wait_action_) {
            AllDetach();
        }
        user_wait_action_ = 0;
        return tmp_action;
    }

    void SetProgress(int now, int max, const std::wstring& msg) {
        if (shell_win_) {
            shell_win_->SetProgress(now, max, msg);
        }
    }
    void SetFileName(const std::wstring& msg) {
        if (shell_win_) {
            shell_win_->SetFileName(msg);
        }
    }
    void SetStatus(const std::wstring& msg) {
        if (shell_win_) {
            shell_win_->SetStatus(msg);
        }
    }

    int ChangeCompleteAndWait() {
        if (shell_win_) {
            //因为窗体在非主线程 使用消息设置完成状态
            ::SendMessage(shell_win_hwnd_, SHELL_WIN_TO_COMPLETE, NULL, NULL);
        }
        return WaitAction();
    }

    std::wstring GetInstallPath() {
        if (shell_win_) {
            return shell_win_->GetInstallPath();
        }
        return L"";
    }

    int ShowMessage(const std::wstring& msg, const std::wstring& caption, int type) {
        if (shell_win_) {
            return shell_win_->ShowMessage(msg, caption, type);
        }
        return MessageBox(NULL, msg.c_str(), caption.c_str(), type);
    }

    void SetProcessCheck(int pc_type, const std::wstring& dir, const std::wstring& ignore, const std::wstring& exe_list) {
        pc_type_ = pc_type;

        pc_dir_ = dir;
        if (pc_dir_.back() == L'\\' || pc_dir_.back() == L'/') {
            pc_dir_.pop_back();
        }
        pc_ignore_ = ignore;
        pc_exe_list_ = exe_list;
    }

    int GetISType() {
        return is_type_;
    }

    bool DllLoading() {
        return dll_loading_;
    }

    void AllDetach() {
        if (shell_win_hwnd_) {
            shell_win_hwnd_ = NULL;
        }
        if (shell_win_) {
            shell_win_.reset();
            //delete shell_win_;
            //shell_win_ = NULL;
        }
        user_wait_action_ = -1;
        dll_loading_ = false;//设置dll标识 自身释放
    }


    void ShowWinThFun() {
        if (!shell_win_) {
            //shell_win_ = new CShellWin;
            shell_win_ = std::make_shared<CShellWin>();
            shell_win_->Create(NULL, _T("ISTShellWinT"), UI_WNDSTYLE_FRAME, UI_WNDSTYLE_EX_FRAME | WS_EX_TOPMOST);
        }
        shell_win_->CustomShow();
        if (shell_win_) {
            shell_win_.reset();
            //delete shell_win_;
            //shell_win_ = NULL;
        }
    }

    void ShowWin() {
        std::thread t([] {
            std::lock_guard<std::mutex> lck(mutex_);
            ShowWinThFun();
        });
        t.detach();
    }

}