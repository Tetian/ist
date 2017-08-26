#include "util.h"
#include <list>
#include <algorithm>//std::transform
#include <Tlhelp32.h>
#include <tchar.h>
#include <process.h>
#include <shlobj.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif
#include <psapi.h> //EnumProcessModules
#pragma comment(lib,"psapi.lib")
namespace util {
    namespace {
#ifndef MAX_PATH_LONG
#define MAX_PATH_LONG 4096
#endif // !MAX_PATH_LONG

        typedef struct tagWNDINFO
        {
            DWORD dwProcessId;
            HWND hWnd;
        } WNDINFO, *LPWNDINFO;

        BOOL CALLBACK ProcessEnumProc(HWND hWnd, LPARAM lParam)
        {
            DWORD dwProcessId;
            GetWindowThreadProcessId(hWnd, &dwProcessId);
            LPWNDINFO pInfo = (LPWNDINFO)lParam;
            if (dwProcessId == pInfo->dwProcessId)
            {
                bool isWindowVisible = TRUE == IsWindowVisible(hWnd);
                if (isWindowVisible == true)
                {
                    pInfo->hWnd = hWnd;
                    return FALSE;
                }

            }
            return TRUE;
        }

        HWND GetProcessMainWnd(DWORD dwProcessId)
        {
            WNDINFO wi;
            wi.dwProcessId = dwProcessId;
            wi.hWnd = NULL;
            EnumWindows(ProcessEnumProc, (LPARAM)&wi);
            return wi.hWnd;
        }



        void SplitStr(std::wstring& s, const std::wstring& delim, std::list<std::wstring>& ret) {
            size_t last = 0;
            size_t index = s.find_first_of(delim, last);
            while (index != std::wstring::npos) {
                ret.push_back(s.substr(last, index - last));
                last = index + 1;
                index = s.find_first_of(delim, last);
            }
            if (index - last > 0) {
                ret.push_back(s.substr(last, index - last));
            }
        }

        std::wstring s2ws(const std::string& s) {
            std::wstring wszStr;
            int nLength = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, NULL);
            wszStr.resize(nLength);
            LPWSTR lpwszStr = new wchar_t[nLength];
            MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, lpwszStr, nLength);
            wszStr = lpwszStr;
            delete[] lpwszStr;
            return wszStr;
        }
        std::string ws2s(const std::wstring& ws) {
            std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
            setlocale(LC_ALL, "chs");
            const wchar_t* _Source = ws.c_str();
            size_t _Dsize = 2 * ws.size() + 1;
            char *_Dest = new char[_Dsize];
            memset(_Dest, 0, _Dsize);
            //wcstombs(_Dest, _Source, _Dsize);//_CRT_SECURE_NO_WARNINGS
            size_t i;
            wcstombs_s(&i, _Dest, _Dsize, _Source, _TRUNCATE);
            std::string result = _Dest;
            delete[]_Dest;
            setlocale(LC_ALL, curLocale.c_str());
            return result;
        }

        std::wstring StrToLower(const std::wstring& str) {
            std::wstring doStr = str;
            std::transform(doStr.begin(), doStr.end(), doStr.begin(), ::tolower);
            return doStr;
        }
        std::string StrToLower(const std::string& str) {
            std::string doStr = str;
            std::transform(doStr.begin(), doStr.end(), doStr.begin(), ::tolower);
            return doStr;
        }
        std::wstring wtrim(const std::wstring& str) {
            std::wstring s_str = str;
            std::wstring::size_type pos = s_str.find_last_not_of(L' ');
            if (pos != std::wstring::npos) {
                s_str.erase(pos + 1);
                pos = s_str.find_first_not_of(L' ');
                if (pos != std::string::npos) s_str.erase(0, pos);
            }
            else
                s_str.erase(s_str.begin(), s_str.end());
            return s_str;
        }

#pragma region ProcessCommand
#define NTAPI __stdcall  
        // NtQueryInformationProcess for pure 32 and 64-bit processes  
        typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
            IN HANDLE ProcessHandle,
            ULONG ProcessInformationClass,
            OUT PVOID ProcessInformation,
            IN ULONG ProcessInformationLength,
            OUT PULONG ReturnLength OPTIONAL
            );

        typedef NTSTATUS(NTAPI *_NtReadVirtualMemory)(
            IN HANDLE ProcessHandle,
            IN PVOID BaseAddress,
            OUT PVOID Buffer,
            IN SIZE_T Size,
            OUT PSIZE_T NumberOfBytesRead);

        // NtQueryInformationProcess for 32-bit process on WOW64  
        typedef NTSTATUS(NTAPI *_NtWow64ReadVirtualMemory64)(
            IN HANDLE ProcessHandle,
            IN PVOID64 BaseAddress,
            OUT PVOID Buffer,
            IN ULONG64 Size,
            OUT PULONG64 NumberOfBytesRead);

        // PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes  
        typedef struct _PROCESS_BASIC_INFORMATION {
            PVOID Reserved1;
            PVOID PebBaseAddress;
            PVOID Reserved2[2];
            ULONG_PTR UniqueProcessId;
            PVOID Reserved3;
        } PROCESS_BASIC_INFORMATION;

        // PROCESS_BASIC_INFORMATION for 32-bit process on WOW64  
        // The definition is quite funky, as we just lazily doubled sizes to match offsets...  
        typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
            PVOID Reserved1[2];
            PVOID64 PebBaseAddress;
            PVOID Reserved2[4];
            ULONG_PTR UniqueProcessId[2];
            PVOID Reserved3[2];
        } PROCESS_BASIC_INFORMATION_WOW64;

        typedef struct _UNICODE_STRING {
            USHORT Length;
            USHORT MaximumLength;
            PWSTR  Buffer;
        } UNICODE_STRING;

        typedef struct _UNICODE_STRING_WOW64 {
            USHORT Length;
            USHORT MaximumLength;
            PVOID64 Buffer;
        } UNICODE_STRING_WOW64;

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)  

        wchar_t* GetPebCommandLine(DWORD pId) {
            NTSTATUS status;
            HANDLE hProcess;
            SYSTEM_INFO si;
            BOOL wow64;
            wchar_t* pCmdLine = NULL;
            do {
                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pId);
                if (hProcess == NULL)break;


                GetNativeSystemInfo(&si);
                IsWow64Process(GetCurrentProcess(), &wow64);


                DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
                DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;
                DWORD pebSize = ProcessParametersOffset + 8;
                DWORD ppSize = CommandLineOffset + 16;
                PBYTE peb = (PBYTE)new BYTE[pebSize];
                PBYTE pUserProcessParameters = (PBYTE)new BYTE[ppSize];
                if (peb == NULL)break;
                if (pUserProcessParameters == NULL)break;
                ZeroMemory(pUserProcessParameters, ppSize);
                ZeroMemory(peb, pebSize);
                if (wow64) {
                    PROCESS_BASIC_INFORMATION_WOW64 pbi;
                    ZeroMemory(&pbi, sizeof(pbi));
                    _NtQueryInformationProcess QueryInformationProcess = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
                    if (QueryInformationProcess == NULL)break;

                    status = QueryInformationProcess(hProcess, 0, &pbi, sizeof(pbi), NULL);
                    if (!NT_SUCCESS(status))break;


                    _NtWow64ReadVirtualMemory64 Wow64ReadVirtualMemory64 = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
                    if (Wow64ReadVirtualMemory64 == NULL)break;
                    status = Wow64ReadVirtualMemory64(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL);
                    if (!NT_SUCCESS(status))break;


                    // read ProcessParameters from 64-bit address space  
                    PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space  
                    status = Wow64ReadVirtualMemory64(hProcess, parameters, pUserProcessParameters, ppSize, NULL);
                    if (!NT_SUCCESS(status))break;


                    // read CommandLine  
                    UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(pUserProcessParameters + CommandLineOffset);
                    pCmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
                    status = Wow64ReadVirtualMemory64(hProcess, pCommandLine->Buffer, pCmdLine, pCommandLine->MaximumLength, NULL);
                    if (!NT_SUCCESS(status))break;
                }
                else
                {
                    // we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS  
                    PROCESS_BASIC_INFORMATION pbi;
                    ZeroMemory(&pbi, sizeof(pbi));

                    // get process information  
                    _NtQueryInformationProcess QueryInformationProcess = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
                    if (QueryInformationProcess == NULL)break;

                    status = QueryInformationProcess(hProcess, 0, &pbi, sizeof(pbi), NULL);
                    if (!NT_SUCCESS(status))break;

                    // read PEB  
                    if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL))
                        break;

                    // read ProcessParameters  
                    PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space  
                    if (!ReadProcessMemory(hProcess, parameters, pUserProcessParameters, ppSize, NULL))
                        break;

                    // read CommandLine  
                    UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(pUserProcessParameters + CommandLineOffset);
                    pCmdLine = (PWSTR)new BYTE[pCommandLine->MaximumLength];
                    if (pCmdLine == NULL)break;
                    ZeroMemory(pCmdLine, pCommandLine->MaximumLength);
                    if (!ReadProcessMemory(hProcess, pCommandLine->Buffer, pCmdLine, pCommandLine->MaximumLength, NULL))
                        break;
                }

            } while (FALSE);


            if (hProcess)
            {
                CloseHandle(hProcess);
            }

            return pCmdLine;
        }
#pragma endregion

        //提权
        bool EnablePriv() {
            HANDLE hToken;
            LUID sedebugnameValue;
            TOKEN_PRIVILEGES tkp;

            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
                //cout << "提权失败。" << endl;
                CloseHandle(hToken);
                return false;
            }

            if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {
                //cout << "提权失败。" << endl;
                CloseHandle(hToken);
                return false;
            }
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Luid = sedebugnameValue;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof tkp, NULL, NULL)) {
                //cout << "提权失败。" << endl;
                CloseHandle(hToken);
                return false;
            }
            //else cout << "提权成功！" << endl;
            CloseHandle(hToken);
            return true;
        }

        bool FindAndKillProcessByName(const std::wstring& p_name) {
            if (p_name.empty()) {
                return false;
            }
            HANDLE handle32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == handle32Snapshot) {
                CloseHandle(handle32Snapshot);
                return false;
            }
            PROCESSENTRY32 pEntry;
            pEntry.dwSize = sizeof(PROCESSENTRY32);
            //搜索进程并终止
            if (Process32First(handle32Snapshot, &pEntry)) {
                BOOL bFound = FALSE;
                if (StrToLower(p_name) == StrToLower(std::wstring(pEntry.szExeFile))) {
                    bFound = TRUE;
                }
                while ((!bFound) && Process32Next(handle32Snapshot, &pEntry)) {
                    if (StrToLower(p_name) == StrToLower(std::wstring(pEntry.szExeFile))) {
                        bFound = TRUE;
                    }
                }
                if (bFound) {
                    CloseHandle(handle32Snapshot);
                    EnablePriv();//提权
                    HANDLE handLe = OpenProcess(PROCESS_TERMINATE, FALSE, pEntry.th32ProcessID);
                    BOOL bResult = TerminateProcess(handLe, 0);
                    CloseHandle(handLe);
                    return bResult == TRUE;
                }
            }
            CloseHandle(handle32Snapshot);
            return false;
        }

        int ProcessExistCount(const std::wstring& p_name_w) {
            int i = 0;
            HANDLE hProcessSnap = NULL;
            PROCESSENTRY32 pe32 = { 0 };
            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == (HANDLE)-1) {
                CloseHandle(hProcessSnap);
                return 0;
            }
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hProcessSnap, &pe32)) {
                do {
                    if (StrToLower(p_name_w) == StrToLower(std::wstring(pe32.szExeFile))) {
                        i++;
                    }
                } while (Process32Next(hProcessSnap, &pe32));
            }
            CloseHandle(hProcessSnap);
            return i;
        }

        bool IsExistedProcess(const std::wstring& p_name) {
            return ProcessExistCount(p_name) >= 1;
        }

        bool WaitForKillProcess(const std::wstring& procecss_name, int sleep_time = 300, int loop_time = 20) {
            int now_time = 0;
            while (IsExistedProcess(procecss_name)) {
                if (now_time > loop_time)return false;//无法关闭
                FindAndKillProcessByName(procecss_name);
                Sleep(sleep_time);
                now_time++;
            }
            return true;//成功关闭
        }

        DWORD GetProcessID(const std::wstring& pro_name, const std::wstring& cmd_line, const std::wstring& no_cl) {
            std::wstring fix_cmd_line = StrToLower(wtrim(cmd_line));
            std::wstring fix_no_cl = StrToLower(wtrim(no_cl));
            HANDLE myhProcess;
            PROCESSENTRY32 mype;
            mype.dwSize = sizeof(PROCESSENTRY32);
            BOOL mybRet;
            //进行进程快照
            myhProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //TH32CS_SNAPPROCESS快照所有进程
            mybRet = Process32First(myhProcess, &mype);
            //循环比较，得出ProcessID
            while (mybRet) {
                if (pro_name == mype.szExeFile) {
                    //是否包含 cmd_line
                    std::wstring c_cmd = StrToLower(wtrim(GetPebCommandLine(mype.th32ProcessID)));
                    if (fix_no_cl.empty() && c_cmd.find(fix_cmd_line) != std::wstring::npos) {
                        CloseHandle(myhProcess);
                        return mype.th32ProcessID;
                    }
                    else {
                        if (!fix_no_cl.empty() && c_cmd.find(fix_no_cl) == std::wstring::npos&&c_cmd.find(fix_cmd_line) != std::wstring::npos) {
                            CloseHandle(myhProcess);
                            return mype.th32ProcessID;
                        }
                        else {
                            mybRet = Process32Next(myhProcess, &mype);
                        }
                    }
                }
                else {
                    mybRet = Process32Next(myhProcess, &mype);
                }
            }
            CloseHandle(myhProcess);
            return 0;
        }

        bool KillProcessByPid(DWORD pid) {
            EnablePriv();//提权
            HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (hProcess == NULL) {
                return false;
            }
            BOOL bRet = ::TerminateProcess(hProcess, 0);
            if (bRet == 0) {//failed
                return false;
            }
            return true;
        }

        std::wstring GetProcessPath(DWORD dwProcessID) {
            std::wstring ret;
            TCHAR Filename[MAX_PATH] = { 0 };
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
            if (hProcess == NULL)return ret;
            HMODULE hModule;
            DWORD cbNeeded;
            if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
                GetModuleFileNameEx(hProcess, hModule, Filename, MAX_PATH);
            }
            else {
                DWORD size = MAX_PATH;
                GetProcessImageFileName(hProcess, Filename, size);
            }
            CloseHandle(hProcess);
            ret = std::wstring(Filename);
            return ret;
        }

        DWORD GetProcessIdByPath(const std::wstring& path) {
            DWORD pid = 0;
            if (!FileExsit(path)) {
                return pid;
            }
            HANDLE handle32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == handle32Snapshot) {
                CloseHandle(handle32Snapshot);
                return pid;
            }
            PROCESSENTRY32 pEntry;
            pEntry.dwSize = sizeof(PROCESSENTRY32);
            //搜索进程
            if (Process32First(handle32Snapshot, &pEntry)) {
                do {
                    std::wstring pPath = GetProcessPath(pEntry.th32ProcessID);
                    if (StrToLower(pPath) == StrToLower(path)) {
                        pid = pEntry.th32ProcessID;
                        CloseHandle(handle32Snapshot);
                    }
                } while (Process32Next(handle32Snapshot, &pEntry));
            }
            CloseHandle(handle32Snapshot);
            return pid;
        }

        int FindAndKillProcessByDir(const std::wstring& dir, bool should_kill, const std::list<std::wstring>& ignore_exe) {
            int ret = 0;
            if (!DirExsit(dir)) return ret;
            WIN32_FIND_DATA FindData;
            std::wstring search_param = dir + L"\\*.exe";
            HANDLE hFind = FindFirstFile(search_param.c_str(), &FindData);
            if (hFind == INVALID_HANDLE_VALUE) {
                FindClose(hFind);
                return ret;
            }

            do {
                // 过虑.和..
                if (_tcscmp(FindData.cFileName, L".") == 0 ||
                    _tcscmp(FindData.cFileName, L"..") == 0) {
                    continue;
                }

                std::wstring f_exe = std::wstring(FindData.cFileName);
                std::wstring exe_path = dir + L"\\" + f_exe;
                if (FileExsit(exe_path)) {
                    DWORD pid = GetProcessIdByPath(exe_path);
                    if (0 != pid) {
                        if (!should_kill)ret = 1;
                        if (should_kill) {
                            bool need_kill = true;
                            for each (std::wstring out_exe in ignore_exe) {
                                if (StrToLower(f_exe) == StrToLower(out_exe)) {
                                    need_kill = false;
                                    break;
                                }
                            }
                            if (need_kill) {
                                if (!KillProcessByPid(pid)) {
                                    ret = 2;
                                }
                            }
                        }
                    }
                }

                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    //遍历子目录
                    std::wstring child_dir = dir + L"\\" + f_exe;
                    int c_ret = FindAndKillProcessByDir(child_dir, should_kill, ignore_exe);
                    if (2 == c_ret) {
                        ret = 2;
                    }
                    else if (!should_kill && 1 == c_ret) {
                        ret = 1;
                    }
                }
            } while (FindNextFile(hFind, &FindData));
            FindClose(hFind);
            return ret;
        }


        BOOL GetNtVersionNumbers(DWORD&dwMajorVer, DWORD& dwMinorVer, DWORD& dwBuildNumber) {
            BOOL bRet = FALSE;
            HMODULE hModNtdll = NULL;
            if (hModNtdll = ::LoadLibraryW(L"ntdll.dll"))
            {
                typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);
                pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;
                pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
                if (pfRtlGetNtVersionNumbers)
                {
                    pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
                    dwBuildNumber &= 0x0ffff;
                    bRet = TRUE;
                }

                ::FreeLibrary(hModNtdll);
                hModNtdll = NULL;
            }

            return bRet;
        }

#define MAX_REG_NAME_LEGTH 255		//386
#define MAX_REG_KEY_LENGTH 255

        bool GetReg(const std::string& s_path, const std::string& s_key, std::string& r_value, HKEY s_root_key) {
            HKEY root_key = s_root_key == NULL ? HKEY_LOCAL_MACHINE : s_root_key;
            HKEY sub_key;

            DWORD return0 = 2;

            std::wstring c_path = s2ws(s_path);
            return0 = RegOpenKeyEx(root_key, c_path.c_str(), NULL, KEY_READ, &sub_key);
            if (ERROR_SUCCESS != return0) {
                RegCloseKey(sub_key);
                RegCloseKey(root_key);
                return false;
            }

            DWORD dwCount = MAX_REG_NAME_LEGTH;
            TCHAR tRegValue[MAX_REG_NAME_LEGTH];
            DWORD dwRegType = REG_BINARY | REG_DWORD | REG_EXPAND_SZ | REG_MULTI_SZ | REG_NONE | REG_SZ;
            std::wstring c_key = s2ws(s_key);
            return0 = RegQueryValueEx(sub_key, c_key.c_str(), NULL, &dwRegType, (LPBYTE)tRegValue, &dwCount);
            if (ERROR_SUCCESS != return0) {
                RegCloseKey(sub_key);
                RegCloseKey(root_key);
                return false;
            }
            r_value = ws2s(tRegValue);
            RegCloseKey(sub_key);
            RegCloseKey(root_key);
            return true;
        }

    }//namespace




    HMODULE GetModuleFromAddr(PVOID p) {
        if (NULL == p) {
            p = GetModuleFromAddr;
        }
        MEMORY_BASIC_INFORMATION m = { 0 };
        VirtualQuery(p, &m, sizeof(MEMORY_BASIC_INFORMATION));
        return (HMODULE)m.AllocationBase;
    }

    void TopMost(bool is_top, HWND hwnd) {
        SetWindowPos(hwnd, is_top ? HWND_TOPMOST : HWND_NOTOPMOST,
            0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    HWND GetProcessHwnd(DWORD pid) {
        return GetProcessMainWnd(pid);
    }

    bool FileExsit(const std::wstring& file) {
        if (file.empty())return false;
        errno_t  ret_code = _waccess_s(file.c_str(), 6);
        bool ret = ret_code == 0;
        return ret;
    }

    bool DirExsit(const std::wstring& dir) {
        bool ret = FILE_ATTRIBUTE_DIRECTORY == PathIsDirectory(dir.c_str());
        return ret;
    }

    bool HideThisProcessWindow() {
        DWORD process_id = GetCurrentProcessId();
        HWND hwnd_p = util::GetProcessHwnd(process_id);
        int loop_time = 0;
        //15*200 3秒超时
        while (!hwnd_p&&loop_time <= 15) {
            hwnd_p = util::GetProcessHwnd(process_id);
            Sleep(200);
            loop_time++;
        }
        if (!hwnd_p) {
            return false;
        }
        if (::IsWindow(hwnd_p) && ::IsWindowVisible(hwnd_p)) {
            ::ShowWindow(hwnd_p, SW_HIDE);
            return true;
        }
        return false;
    }

    std::wstring GetPathFileName(const std::wstring& path) {
        std::wstring tpath = path;
        if (tpath.empty())return TEXT("");
        //去掉结尾\或/
        if (tpath.back() == L'\\' || tpath.back() == L'/') {
            tpath.pop_back();
        }
        size_t pos = tpath.find_last_of('\\');
        if (pos == std::wstring::npos) {
            pos = tpath.find_first_not_of('/');
            if (pos == std::wstring::npos)return TEXT("");
        }
        std::wstring ret(tpath.substr(pos + 1));
        return ret;
    }

    int ProcessExisted(const std::wstring& pro_name, int should_kill) {
        if (0 == should_kill) {
            return IsExistedProcess(pro_name) ? 1 : 0;
        }
        else if (1 == should_kill) {
            return WaitForKillProcess(pro_name) ? 1 : 0;
        }
        return 0;
    }

    int ProcessExistedList(const std::wstring& pro_name_s, int should_kill) {
        std::list<std::wstring> plist;
        SplitStr(std::wstring(pro_name_s), L",", plist);
        int ret = 0;
        if (0 == should_kill) {
            for each (std::wstring p in plist) {
                if (IsExistedProcess(p)) {
                    ret = 1;
                    break;
                }
            }
        }
        else if (1 == should_kill) {
            std::list<std::wstring>::iterator iter = plist.begin();
            for (iter; iter != plist.end();) {
                std::wstring pro_name = (*iter);
                if (IsExistedProcess(pro_name)) {
                    if (!WaitForKillProcess(pro_name)) {
                        if (ret != 2)ret = 2;
                    }
                }
                else ++iter;
            }
        }
        return ret;
    }

    int ProcessExistedDir(const std::wstring& dir, int should_kill, const std::wstring& p_ignore_s) {
        std::list<std::wstring> exe_list;
        std::list<std::wstring> out_list;
        SplitStr(std::wstring(p_ignore_s), L",", out_list);
        return FindAndKillProcessByDir(dir, 1 == should_kill, out_list);
    }

    int ProcessExistedByCL(const std::wstring& pro_name, int should_kill, const std::wstring cl) {
        DWORD pid = GetProcessID(pro_name, cl, L"");
        if (0 == should_kill) {
            return pid > 0 ? 1 : 0;
        }
        else if (1 == should_kill) {
            if (pid > 0) {
                return KillProcessByPid(pid) ? 1 : 0;
            }
        }
        return 0;
    }

    int ProcessExistedNoCL(const std::wstring& pro_name, int should_kill, const std::wstring cl) {
        DWORD pid = GetProcessID(pro_name, L"", cl);
        if (0 == should_kill) {
            return pid > 0 ? 1 : 0;
        }
        else if (1 == should_kill) {
            if (pid > 0) {
                return KillProcessByPid(pid) ? 1 : 0;
            }
        }
        return 0;
    }

    int ProcessExisted(int type, const std::wstring& param1, int should_kill, const std::wstring& param2) {
        if (param1.empty()) {
            return 0;
        }
        if (0 == type) {
            return ProcessExisted(param1, should_kill);
        }
        else if (1 == type) {
            return ProcessExistedList(param1, should_kill);
        }
        else if (2 == type) {
            return ProcessExistedDir(param1, should_kill, param2);
        }
        else if (3 == type) {
            return ProcessExistedByCL(param1, should_kill, param2);
        }
        else if (4 == type) {
            return ProcessExistedNoCL(param1, should_kill, param2);
        }
        return 0;
    }

    bool IsValidDisk(const std::wstring& path) {
        std::wstring t_path = path;
        wchar_t* szBuffer = (wchar_t*)t_path.c_str();
        if (TRUE == PathStripToRoot(szBuffer)) {
            unsigned int type = GetDriveType(t_path.c_str());
            return DRIVE_FIXED == type;
            //std::wstring tn;
            /* switch (type) {
            case DRIVE_UNKNOWN:
            tn = _T("未知类型");
            break;
            case DRIVE_NO_ROOT_DIR:
            tn = _T("无效的根路径");
            break;
            case DRIVE_REMOVABLE:
            tn = _T("可移动磁盘或软盘");
            break;
            case DRIVE_FIXED:
            tn = _T("本地硬盘");
            break;
            case DRIVE_REMOTE:
            tn = _T("网络磁盘");
            break;
            case DRIVE_CDROM:
            tn = _T("CD-ROM");
            break;
            case DRIVE_RAMDISK:
            tn = _T("RAM");
            break;
            default:
            tn = _T("未知类型");
            }*/
        }
        return false;
    }

    std::wstring GetLocalAppDataPath() {
        wchar_t m_lpszDefaultDir[MAX_PATH_LONG];
        wchar_t szDocument[MAX_PATH_LONG] = { 0 };
        memset(m_lpszDefaultDir, 0, MAX_PATH_LONG);
        LPITEMIDLIST pidl = NULL;
        SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl);
        if (pidl && SHGetPathFromIDList(pidl, szDocument)) {
            GetLongPathName(szDocument, m_lpszDefaultDir, MAX_PATH_LONG);
        }
        std::wstring wsR(m_lpszDefaultDir);
        return wsR;
    }

    bool IsWinXP() {
        DWORD dwMajorVer;
        DWORD dwMinorVer;
        DWORD dwBuildNumber;

        if (TRUE == GetNtVersionNumbers(dwMajorVer, dwMinorVer, dwBuildNumber)) {
            if (5 == dwMajorVer && 1 == dwMinorVer) {
                return true;
            }
        }
        return false;
    }

    bool IsWinXPSP3orGreater() {
        DWORD dwMajorVer;
        DWORD dwMinorVer;
        DWORD dwBuildNumber;
        if (TRUE == GetNtVersionNumbers(dwMajorVer, dwMinorVer, dwBuildNumber)) {
            if (5 < dwMajorVer) {   
                return true;
            }
            else if (5 == dwMajorVer && 1 == dwMinorVer) {
                std::string v;
                bool is_get_v = GetReg("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CSDVersion", v, HKEY_LOCAL_MACHINE);
                if (is_get_v) {
                    v = StrToLower(v);
                    if (v.find(StrToLower("Service Pack 3")) != std::string::npos) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}