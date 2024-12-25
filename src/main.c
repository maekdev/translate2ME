// main.c
///
// 240916
// markus ekler
//
// Rework of DrWortklauber using wchar only

#include "config.h"
#include "cfg_file.h"
#include "hwnd.h"
#include "hotkey.h"

HANDLE hMutex;
HWND hwndmain;

#ifdef _DEBUG
void _PRINTF(const wchar_t *format, ...) {
    va_list arglist;
    
    va_start(arglist, format);
    vwprintf(format, arglist);
    va_end(arglist);
}
#endif // _DEBUG

int MutexCreate(void) {
    DWORD dwReturn;
    hMutex = CreateMutexW(NULL,FALSE,MUTEXNAME);
    dwReturn = WaitForSingleObject(hMutex,1000);
    if ((dwReturn == WAIT_OBJECT_0) || (dwReturn == WAIT_ABANDONED)) {
        return 0;
    }
    return 1;
}

int MutexDestroy(void) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
}

int InitApplication(HINSTANCE hinst) {
    t_cfgmain *cfgm;
    
    // Mutex... es kann nur einen geben!
    if (MutexCreate()) {
		_PRINTF(L"InitApplication(): MutexCreate() failed.\n");
        MessageBoxW(NULL,L"translate2ME was started already.\nYou can run only one instance.",L"Error - InitApplication()",MB_OK|MB_ICONERROR);
        return 1;
    }
	
    _PRINTF(L"InitApplication(): load config files.\n");
    cfgmain_Init();	
    cfgmain_ReadFile();
	
    _PRINTF(L"InitApplication(): create mainwindow.\n");
    hwndmain = hwnd_Init();
    if (!hwndmain) 
        return 2;
    
    _PRINTF(L"InitApplication(): register hotkeys.\n");
    if (Hotkey_Init(hwndmain) != 0)
        return 3;
        
	_PRINTF(L"InitApplication(): load last config.\n");
    cfgm = (t_cfgmain*)cfgmain_Get();
    
	if (cfgm->lang_cfgfile[cfgm->lang_last] != NULL) {
        cfglang_ReadFile(cfgm->lang_cfgfile[cfgm->lang_last]);
    } else {
        if (cfgm->lang_cfgfile[0] == NULL) {
            _PRINTF(L"InitApplication(): Didn't find language files.\n");
            MessageBoxW(NULL,L"No language files found.\nYou can use this program only with dictionary files.",L"Error - InitApplication()",MB_OK|MB_ICONERROR);
        }
        cfglang_ReadFile(cfgm->lang_cfgfile[0]);
    }	
    return 0;
}

int DestroyApplication(void) {

    _PRINTF(L"DestroyApplication()\n");

    // hotkey
    Hotkey_Destroy(hwndmain);
    
    // main wnd
    DestroyWindow(hwndmain);    
    
    // konfiguration
    cfgmain_Destroy();
    
    // Mutex freigeben
    MutexDestroy();
    
    // shutdown
    PostQuitMessage(0); 

    return 0;
}

//int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nCmdShow) {
int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nCmdShow) {
    
    MSG msg;
    HACCEL haccel;
       
    if (InitApplication(hInstance) != 0)
        return FALSE;

    haccel = LoadAcceleratorsW(GetModuleHandle(NULL),L"FontAccel");
    if (haccel == NULL) {
        MessageBox(NULL,L"Keyboard combination CTRL+V not available",L"LoadAccelerators() ERROR",MB_OK|MB_ICONERROR);
    }
    
    // main message loop
    while (GetMessage(&msg,NULL,0,0)) {
        if (!TranslateAccelerator(hwndmain,haccel,&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return (int)msg.wParam;
}
