#include <Windows.h>
#include "util.h"
#include "shell.h"
#include "duilib_ori\UIlib.h"

void onDLLProcessAttach(HMODULE hModule){
    //初始duilib全局实例句柄
	DuiLib::CPaintManagerUI::SetInstance(hModule);
}

void onDLLThreadAttach(){
}

void onDLLThreadDetach(){

}

void onDLLProcessDetach(){
    //Zip资源设置缓存true
    DuiLib::CPaintManagerUI::Term();
}


BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved)
{
	switch( ul_reason_for_call ) 
	{
		case DLL_PROCESS_ATTACH:
			onDLLProcessAttach(hModule);
			break;
		case DLL_THREAD_ATTACH:
			onDLLThreadAttach();
			break;
		case DLL_THREAD_DETACH:
			onDLLThreadDetach();
			break;
		case DLL_PROCESS_DETACH:
			onDLLProcessDetach();
			break;
	}
	return TRUE;
}