#include "ui_message_box.h"
//引入duilib
#include "duilib_ori\UIlib.h"
namespace ui {
    using namespace DuiLib;
    namespace {
        UINT MessageBoxResId = 0;
        std::wstring MessageBoxFile;
    }

    class MessageBoxWin : public WindowImplBase {
    public:
        MessageBoxWin(const std::wstring& content, const std::wstring& caption, UINT type, unsigned int* ret_id) :
            content_(content), caption_(caption), type_(type), ret_id_(ret_id) {
        }
        ~MessageBoxWin() {}

        virtual CDuiString GetSkinFolder() override {
            return _T("");
        }

        virtual CDuiString GetSkinFile() override {
            if (0 != MessageBoxResId) {
                TCHAR szBuf[MAX_PATH] = { 0 };
                _stprintf_s(szBuf, MAX_PATH - 1, _T("%d"), MessageBoxResId);
                return szBuf;
            }
            else if (!MessageBoxFile.empty()) {
                return MessageBoxFile.c_str();
            }
            else {
                return _T("message_box.xml");
            }
        }

        virtual LPCTSTR GetWindowClassName(void) const  override {
            return _T("TMessageBoxWindowUI");
        }
        virtual void InitWindow() override {
            this->CenterWindow();
            CControlUI* pControl;

            if (!caption_.empty()) {
                pControl = m_PaintManager.FindControl(_T("caption"));
                if (pControl)
                    pControl->SetText(caption_.c_str());
            }

            if (!content_.empty()) {
                pControl = m_PaintManager.FindControl(_T("content"));
                if (pControl)
                    pControl->SetText(content_.c_str());
            }

            if (MB_OK == type_) {
                pControl = m_PaintManager.FindControl(_T("btnCancel"));
                if (pControl)
                    pControl->SetVisible(false);

                pControl = m_PaintManager.FindControl(_T("btnOk"));
                if (pControl) {
                    SIZE pos = pControl->GetFixedXY();
                    pos.cx += 60;
                    pControl->SetFixedXY(pos);
                }
            }
        }

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override {
            if (uMsg == WM_KEYDOWN) {
                if (wParam == VK_RETURN) {
                    //::PostMessage(m_hWnd, WM_CLOSE, IDOK, 0);
                    *ret_id_ = IDOK;
                    this->Close(IDOK);
                }
                else if (wParam == VK_ESCAPE) {
                    //::PostMessage(m_hWnd, WM_CLOSE, IDCANCEL, 0);
                    *ret_id_ = IDCANCEL;
                    this->Close(IDCANCEL);
                }
            }
            return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
        }
        LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override {
            if (uMsg == WM_KEYDOWN) {
                switch (wParam) {
                case VK_RETURN:                        // 回车建 
                    return FALSE;
                    break;
                case VK_ESCAPE:                        // ESC 
                    return FALSE;
                case VK_SPACE:                         // 空格键
                    return FALSE;
                default:
                    break;
                }
            }
            return WindowImplBase::MessageHandler(uMsg, wParam, lParam, bHandled);
        }

        void Notify(TNotifyUI& msg) override {
            if (_tcsicmp(msg.sType, _T("click")) == 0) {
                if (_tcsicmp(msg.pSender->GetName(), _T("btnOk")) == 0) {
                    //::PostMessage(m_hWnd, WM_CLOSE, IDOK, 0);
                    *ret_id_ = IDOK;
                    this->Close(IDOK);
                }
                else if (_tcsicmp(msg.pSender->GetName(), _T("btnCancel")) == 0) {
                    //::SendMessage(m_hWnd, WM_CLOSE, IDCANCEL, 0);
                    *ret_id_ = IDCANCEL;
                    this->Close(IDCANCEL);
                }
                else if (_tcsicmp(msg.pSender->GetName(), _T("btnClose")) == 0) {
                    //::PostMessage(m_hWnd, WM_CLOSE, IDCANCEL, 0);
                    *ret_id_ = IDCANCEL;
                    this->Close(IDCANCEL);
                }
            }
            WindowImplBase::Notify(msg);
        }
        UINT GetClassStyle() const override {
            return CS_DBLCLKS;
        }
        void OnFinalMessage(HWND hWnd) override {
            m_PaintManager.RemoveNotifier(this);
            m_PaintManager.ReapObjects(m_PaintManager.GetRoot());
            delete this;
        }


        UINT ShowModalEx() {
            ASSERT(::IsWindow(m_hWnd));
            UINT nRet = 0;
            HWND hWndParent = GetWindowOwner(m_hWnd);
            ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
            ::EnableWindow(hWndParent, FALSE);
            MSG msg = { 0 };
            while (::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0)) {
                if (msg.message == WM_CLOSE && msg.hwnd == m_hWnd) {
                    nRet = msg.wParam;
                    ::EnableWindow(hWndParent, TRUE);
                    ::SetFocus(hWndParent);
                }
                if (!CPaintManagerUI::TranslateMessage(&msg)) {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
                if (msg.message == WM_QUIT) break;
            }
            ::EnableWindow(hWndParent, TRUE);
            ::SetFocus(hWndParent);
            if (msg.message == WM_QUIT) {
                ::PostQuitMessage(msg.wParam);
            }
            return nRet;
        }
    private:
        UINT type_;
        std::wstring caption_, content_;
        unsigned int* ret_id_;
    };

    void SetMessageBoxRes(UINT res_id) {
        MessageBoxResId = res_id;
    }
    void SetMessageBoxRes(const std::wstring& res_file) {
        MessageBoxFile = res_file;
    }

    unsigned int MessageBoxU(HWND hWnd, const std::wstring&  content, const std::wstring& caption, UINT type) {
        unsigned int ret = 0;
        MessageBoxWin* wnd = new MessageBoxWin(content, caption, type, &ret);
        if (wnd&&wnd->Create(hWnd, _T("TMessageBoxWindowUI"), UI_WNDSTYLE_DIALOG, NULL)) {
            wnd->ShowModal();
        }
        return ret;
    }

}