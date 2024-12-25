// hotkey.c
//
// 130722
// markus ekler
// 


#include "config.h"
#include "hwnd.h"
#include "cfg_file.h"

#define HOTKEY_ID       5001

int Hotkey_Init(HWND hwndmain) {
    t_cfgmain *cfgm;
    
    cfgm = (t_cfgmain*)cfgmain_Get();	
    RegisterHotKey(hwndmain,HOTKEY_ID,cfgm->hotkey_mod,cfgm->hotkey_id);
	
    return 0;
}

int Hotkey_Destroy(HWND hwndmain) {
    UnregisterHotKey(hwndmain,HOTKEY_ID);
    
    return 0;
}

int Hotkey_Process(HWND hDlg,WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case 5001:
            hwnd_Toggle();
            break;
        }    
    return 0;
}
