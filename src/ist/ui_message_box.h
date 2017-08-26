#pragma once
#include <windows.h>
#include <string>
namespace ui {
    unsigned int MessageBoxU(HWND hWnd, const std::wstring& content, const std::wstring&  caption, UINT type = MB_OK);
    void SetMessageBoxRes(UINT res_id);
    void SetMessageBoxRes(const std::wstring& res_file);
}


