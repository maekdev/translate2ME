#ifndef __HOTKEY_H
#define __HOTKEY_H

int Hotkey_Init(HWND hwndmain);
int Hotkey_Destroy(HWND hwndmain);
int Hotkey_Process(HWND hDlg,WPARAM wParam);

#endif // __HOTKEY_H
