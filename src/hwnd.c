// hwnd.c
//
// 240917
// markus ekler
//
// rewritten version for UNICODE support

#include "config.h"
#include "hwnd.h"
#include "cfg_file.h"
#include "hotkey.h"
#include "search.h"

// global data
HWND hwnd = NULL;
wchar_t szClassName[ ] = L"WortklauberWindow";

HDC hdcmem = NULL;
HBITMAP hbmmem;

wchar_t sstr[SSTRLEN];

t_ergebnis *erg;
int erg_pos=0;


// string entering functions
void str_push(wchar_t *str,wchar_t ch) {
    if (wcslen(str) >= SSTRLEN-1) 
        return;
    while (*str) {
        str++;
    }
    *str = ch;
    *(str+1) = '\0';
}

void str_clear(wchar_t *str) {
    str[0] = '\0';
}

void str_backspace(wchar_t *str) {
    if (wcslen(str) == 0) 
        return;
    while (*str)
        str++;
    *(str-1) = '\0';
}
    
// functions
void hwnd_InputUpdate(int dnr) {
    // dnr = delete and redraw (0=keep n overwrite)
    HDC hdchwnd;
    t_cfgmain *cfgm;
    RECT r;
    RECT cr;
    HBRUSH hb;
    HFONT hfont;
    int i;
    int fs_calc=80; // default input fontsize for testing
    wchar_t txt[1024];
    
    // load cfgmain 
    cfgm = (t_cfgmain*)cfgmain_Get();
    fs_calc = cfgm->inputfontsize;
    
    // calculate window size
    hdchwnd = GetDC(hwnd);
    GetClientRect(hwnd,&r);    
    r.bottom = cfgm->input_height;    
    while (fs_calc >= 8) { // 8 as minimum fontsize
        GetClientRect(hwnd,&cr);
        cr.bottom = r.bottom;
        hfont = CreateFont(fs_calc, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
        SelectObject(hdchwnd,hfont);
		//DrawText(hdchwnd,sstr,strlen(sstr),&cr,DT_CALCRECT|DT_SINGLELINE|DT_TOP|DT_LEFT);
        DrawTextW(hdchwnd,sstr,wcslen(sstr),&cr,DT_CALCRECT|DT_SINGLELINE|DT_TOP|DT_LEFT);
        DeleteObject(hfont);
        if (cr.right > (r.right-IMAGE_WIDTH-10)) {
            dnr = 1;
            fs_calc--;            
        } else {
            break;
        }
    } 
    
    if (dnr) {        
        hb = CreateSolidBrush(RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
        FillRect(hdchwnd,&r,hb);
        DeleteObject(hb);
        
        // display bitmap
        if (cfgm->hdcbmp != NULL) {
            GetClientRect(hwnd,&cr);            
            BitBlt(hdchwnd,cr.right-IMAGE_WIDTH-1,0,IMAGE_WIDTH,cfgm->input_height,cfgm->hdcbmp,0,0,SRCCOPY);
        }
    }
    
    // draw search text
    hfont = CreateFont(fs_calc, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
    SelectObject(hdchwnd,hfont);
    SetBkColor(hdchwnd,RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    r.top += 10;
    r.left += 10;
    DrawTextW(hdchwnd,sstr,wcslen(sstr),&r,DT_SINGLELINE|DT_TOP|DT_LEFT);
    DeleteObject(hfont);
        
    ReleaseDC(hwnd,hdchwnd);
}

void hwnd_ConfigUpdate(void) {
    HDC hdchwnd;    
    RECT r;
    RECT r_tmp;
    
    t_cfgmain *cfgm;
    HFONT hfont;
    HBRUSH hb;
    wchar_t txt[1024];
    int i;
    
    // load cfgmain
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // DC handling
    hdchwnd = GetDC(hwnd);
    
    // clear client area
    GetClientRect(hwnd,&r);
    r.top = cfgm->input_height;
    hb = CreateSolidBrush(RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    FillRect(hdchwnd,&r,hb);
    DeleteObject(hb);
    
    // display languages
    GetClientRect(hwnd,&r);
    r.left = 10;
    r.top = cfgm->input_height + 10;
    r.right /= 2;
    r.bottom -= 10;    
    
    hfont = CreateFont(cfgm->textfontsize, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
    SelectObject(hdchwnd,hfont);
    SetBkColor(hdchwnd,RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    //txt[0] = '\0';
    wsprintf(txt,L"<ENTER> : Search\n<ALT>+<ENTER> : WWW Lookup\n<ALT>+<BACKSPACE> : Clear Search\n<ESC> : Hide Window\n\n");
    for (i=0;i!=9;i++) {
        if (cfgm->lang_cfgname[i] == NULL) 
            break;
        wsprintf(txt,L"%sALT+%i : %s\n",txt,i+1,cfgm->lang_cfgname[i]);
    }
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_TOP|DT_LEFT);
    
    // right column
    GetClientRect(hwnd,&r);
    r.left = r.right/2 + 10;
    r.right -= 10;
    r.top = cfgm->input_height + 10;
    r.bottom -= 10;
    CopyRect(&r_tmp,&r);
    // first line substitutions
    wsprintf(txt,L"ALT+J : Language-specific Substitution ON OFF");
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    SetTextColor(hdchwnd,RGB(cfgm->fonthl_r,cfgm->fonthl_g,cfgm->fonthl_b));
    if (cfgm->search_subst) {
        wsprintf(txt,L"ALT+J : Language-specific Substitution ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"ON",2,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    } else {
        wsprintf(txt,L"ALT+J : Language-specific Substitution ON ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"OFF",3,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    }
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    // second line (case sensitivity)
    r_tmp.top += cfgm->textfontsize;
    CopyRect(&r,&r_tmp);
    wsprintf(txt,L"ALT+K : Case-Matching IGNORE MATCH");
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    SetTextColor(hdchwnd,RGB(cfgm->fonthl_r,cfgm->fonthl_g,cfgm->fonthl_b));
    if (cfgm->search_ignorecase) {
        wsprintf(txt,L"ALT+K : Case-Matching ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"IGNORE",6,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    } else {
        wsprintf(txt,L"ALT+K : Case-Matching IGNORE ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"MATCH",5,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    }
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    // third line (literal/regex/wildcard)
    r_tmp.top += cfgm->textfontsize;
    CopyRect(&r,&r_tmp);
    wsprintf(txt,L"ALT+L : Literal- / REGEX- / Wildcard- Search");
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    SetTextColor(hdchwnd,RGB(cfgm->fonthl_r,cfgm->fonthl_g,cfgm->fonthl_b));
    if (cfgm->search_search == 0) {
        wsprintf(txt,L"ALT+L : ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"Literal-",8,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    } else if (cfgm->search_search == 1) {
        wsprintf(txt,L"ALT+L : Literal- / ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"REGEX-",6,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    } else {
        wsprintf(txt,L"ALT+L : Literal- / REGEX- / ");
        DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
        r.left = r.right;
        r.right = r_tmp.right;
        DrawTextW(hdchwnd,L"Wildcard-",9,&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
    }    
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    
    r_tmp.top += 2*(cfgm->textfontsize);
    CopyRect(&r,&r_tmp);
    wsprintf(txt,L"<PAGE UP> : Previous Search Result\n<PAGE DOWN> : Next Search Result\n\n\nALT+Q : Close Program");
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_TOP|DT_LEFT);

    // greeting message
    DeleteObject(hfont);
    hfont = CreateFont(10, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
    SelectObject(hdchwnd,hfont);
    wsprintf(txt,INFO_STRING);
    GetClientRect(hwnd,&r);
    DrawTextW(hdchwnd,txt,wcslen(txt),&r,DT_BOTTOM|DT_RIGHT|DT_SINGLELINE);
    
    DeleteObject(hfont);
    ReleaseDC(hwnd,hdchwnd);
}

void hwnd_ConfigShow(void) {
    // display key combos / config screen upon ALT
    HDC hdchwnd;    
    RECT r;
    
    if (hdcmem != NULL) {
        _PRINTF(L"WARNING: hdcmem!=NULL\n");
        DeleteObject(hbmmem);
        DeleteDC(hdcmem);
    }
        
    // create snapshot
    hdchwnd = GetDC(hwnd);
    GetClientRect(hwnd, &r);
    hdcmem = CreateCompatibleDC(hdchwnd);
    hbmmem = CreateCompatibleBitmap(hdchwnd,r.right-r.left,r.bottom-r.top);
    SelectObject(hdcmem,hbmmem);
    BitBlt(hdcmem,0,0,r.right-r.left,r.bottom-r.top,hdchwnd,0,0,SRCCOPY);

    // DC release
    ReleaseDC(hwnd,hdchwnd);    
    
    // display window contents
    hwnd_ConfigUpdate();
}

void hwnd_ConfigHide(void) {
    // restore window / hide config and release DC
    HDC hdchwnd;
    RECT r;
    t_cfgmain *cfgm;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    if (hdcmem == NULL) {
        _PRINTF(L"ERR: hdcmem=NULL\n");
        return ;
    }
    
    hdchwnd = GetDC(hwnd);
    GetClientRect(hwnd, &r);    
    
    // only restore lower window
    BitBlt(hdchwnd,0,cfgm->input_height,r.right-r.left,r.bottom-r.top,hdcmem,0,cfgm->input_height,SRCCOPY);    
    
    ReleaseDC(hwnd,hdchwnd);
    
    DeleteObject(hbmmem);
    DeleteDC(hdcmem);
    hdcmem = NULL;
}

void hwnd_ResultStrShow(wchar_t *str) {
    HDC hdchwnd;
    RECT r;
    RECT r_tmp;
    HFONT hfont;
    HBRUSH hb;
    t_cfgmain *cfgm;
    
    // load cfgmain
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // hdc
    hdchwnd = GetDC(hwnd);
    
    // clear client area
    GetClientRect(hwnd,&r);
    r.top = cfgm->input_height;
    hb = CreateSolidBrush(RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    FillRect(hdchwnd,&r,hb);
    DeleteObject(hb);
    
    // font stuff
    hfont = CreateFont(cfgm->textfontsize, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
    SelectObject(hdchwnd,hfont);
    SetBkColor(hdchwnd,RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    SetTextColor(hdchwnd,RGB(cfgm->fonthl_r,cfgm->fonthl_g,cfgm->fonthl_b));
    
    // calculate available area
    GetClientRect(hwnd,&r);
    r.top = cfgm->input_height + 10;
    r.left += 10;
    r.bottom -= 10;
    r.right -= 10;
    
    DrawTextW(hdchwnd,str,wcslen(str),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);

    // clean up
    DeleteObject(hfont);
    ReleaseDC(hwnd,hdchwnd);
}

void hwnd_ResultShow(t_ergebnis *erg) {
    HDC hdchwnd;
    RECT r;
    RECT r_tmp;
    HFONT hfont;
    HBRUSH hb;
    t_cfgmain *cfgm;
    wchar_t *str;
    int i;
    
    // load config
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // hdc
    hdchwnd = GetDC(hwnd);
    
    // clear client area
    GetClientRect(hwnd,&r);
    r.top = cfgm->input_height;
    hb = CreateSolidBrush(RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    FillRect(hdchwnd,&r,hb);
    DeleteObject(hb);
    
    // font
    hfont = CreateFont(cfgm->textfontsize, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, FF_DONTCARE, NULL);
    SelectObject(hdchwnd,hfont);
    SetBkColor(hdchwnd,RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));
    SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));
    
    // calculate available area
    GetClientRect(hwnd,&r);
    r.top = cfgm->input_height + 10;
    r.left += 10;
    r.bottom -= 10;
    r.right -= 10;
    
    // display
    i = 0;
    while (erg != NULL && i!=erg_pos) {
        erg = erg->next;
        i++;
        if (search_Count(erg) <= cfgm->resultsmin) {
            break;
        }
    }
    while (erg != NULL) {
        str = (cfgm->data)+(erg->abs_start);
        DrawTextW(hdchwnd,str,wcslen(str),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
        CopyRect(&r_tmp,&r);
        // highlight
        SetTextColor(hdchwnd,RGB(cfgm->fonthl_r,cfgm->fonthl_g,cfgm->fonthl_b));
        if (erg->offset_start != erg->abs_start) {
            DrawTextW(hdchwnd,str,erg->offset_start-erg->abs_start,&r,DT_CALCRECT|DT_TOP|DT_LEFT|DT_SINGLELINE);
            r.left = r.right;
            r.right = r_tmp.right;        
            str = (cfgm->data)+(erg->offset_start);            
        }
        DrawTextW(hdchwnd,str,(erg->offset_stop-erg->offset_start),&r,DT_TOP|DT_LEFT|DT_SINGLELINE);
        // restore
        SetTextColor(hdchwnd,RGB(cfgm->font_r,cfgm->font_g,cfgm->font_b));        
        CopyRect(&r,&r_tmp);
        r.top += cfgm->textfontsize;
        if (r.top >= (r.bottom - cfgm->textfontsize)) 
            break;
        erg = erg->next;
    }
    
    // clean up
    DeleteObject(hfont);
    ReleaseDC(hwnd,hdchwnd);
}

void OpenSearchLink(void) {
	// quick n dirty function to open search link on www
    t_cfgmain *cfgm;
    wchar_t stmp[1024];
    wchar_t *ttmp;
    wchar_t ssearch[] = L"<SEARCH>";
    int i,o;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    ttmp = wcsstr(cfgm->searchurl,ssearch);
    if (ttmp == NULL) {
        _PRINTF(L"OpenSearchLink(): %s\n",cfgm->searchurl);
        ShellExecuteW(NULL,L"open",cfgm->searchurl,NULL,NULL,SW_SHOWNORMAL);
    } else {
        for (i=0;i<1024&&i!=(ttmp-(cfgm->searchurl));i++) {
            stmp[i] = *(cfgm->searchurl+i);
        }
        stmp[i] = '\0';
        wcscat(stmp,sstr);
        wcscat(stmp,ttmp+wcslen(ssearch));
        _PRINTF(L"OpenSearchLink(): %s\n",stmp);
        ShellExecuteW(NULL,L"open",stmp,NULL,NULL,SW_SHOWNORMAL);
    }
}

int ProcessInputCombo(WPARAM wParam) {
    HDC hdc;
    PAINTSTRUCT psPaint;
    RECT r;
    t_cfgmain *cfgm;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    switch (wParam) {
        case 0x31: // 1
        case 0x32: 
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39: // 9
            if (cfgm->lang_cfgfile[wParam-0x31] != NULL) {
                if (erg != NULL) {
                    erg_Delete(erg);
                    erg_pos = 0;
                    erg = NULL;
                }
                _PRINTF(L" == Loading LANG file (%ls) ==\n",cfgm->lang_cfgname[wParam-0x31]);
                cfglang_ReadFile(cfgm->lang_cfgfile[wParam-0x31]);
                hwnd_InputUpdate(1);
                cfgm->lang_last = wParam-0x31;
            }
            break;
        case 0x0D: // ENTER
            OpenSearchLink();
            break;
        case 0x08: // Backspace
            str_clear(sstr);
            hwnd_InputUpdate(1);
            break;
        case 0x51: // Q
            _PRINTF(L"Shutdown. (ALT+Q) received.\n");
            DestroyApplication();
            break;
        //case 0x53: // S - substitution options ON/OFF
        case 0x4A: // J
            if (cfgm->search_subst) {
                cfgm->search_subst = 0;
                _PRINTF(L"Language Substitution OFF\n");
            } else {
                cfgm->search_subst = 1;
                _PRINTF(L"Language Substitution ON\n");
            }
            hwnd_InputUpdate(1);
            hwnd_ConfigUpdate();
            break;
        //case 0x43: // C - case sensitivity ON/OFF
        case 0x4B: // K
            if (cfgm->search_ignorecase) {
                cfgm->search_ignorecase = 0;
                _PRINTF(L"Case Sensitivity ON\n");
            } else {
                cfgm->search_ignorecase = 1;
                _PRINTF(L"Case Sensitivity OFF\n");
            }
            hwnd_InputUpdate(1);
            hwnd_ConfigUpdate();
            break;
        //case 0x52: // R - Regex entry ON/OFF            
        case 0x4C: // L
            cfgm->search_search += 1;
            cfgm->search_search %= 3;
            switch (cfgm->search_search) {
                case 0: _PRINTF(L"LITERAL search\n"); break;
                case 1: _PRINTF(L"REGEX search\n"); break;
                default: _PRINTF(L"WILDCARD serach\n"); break;
            }
            hwnd_InputUpdate(1);
            hwnd_ConfigUpdate();
            break;
        /*case 0x41: // A - Approximierte Suche AN/AUS
            if (cfgm->search_approx) {
                cfgm->search_approx = 0;
                _PRINTF("Direkte Suche\n");
            } else {
                cfgm->search_approx = 1;
                _PRINTF("Approximierte Suche\n");
            }
            hwnd_InputUpdate(1);
            break;
        */
    }
    return 0;
}

int ProcessInput(WPARAM wParam) {
    int errcode;    
    
    if (wParam == VK_ESCAPE) {
        //if (strlen(sstr) == 0) {
            hwnd_Toggle();
        //} else {
        //    str_clear(sstr);
        //}    
    } else if (wParam == VK_RETURN) {
        if (wcslen(sstr) > 0) {
            _PRINTF(L" ===>>> START SEARCH\n");
            errcode = search_Create(sstr);
            if (errcode) {
                hwnd_ResultStrShow(search_ErrDecode(errcode));
                _PRINTF(L"ERROR: %s\n",search_ErrDecode(errcode));
            } else {
                if (erg != NULL) {
                    erg_Delete(erg);
                    erg_pos = 0;
                    erg = NULL;
                }
                erg = search_Search();
                search_Sort(erg);
                if (search_Count(erg) == 0) {
                    hwnd_ResultStrShow(L"No results found.");
                } else {
                    hwnd_ResultShow(erg);
                }
            }
            search_Destroy();
        }
    } else if (wParam == VK_BACK) {
        str_backspace(sstr);
    } else { //if ((wParam >= 0x30 && wParam <= 0x39) || (wParam >= 0x41 && wParam <= 0x5A)) { 
        // character entry (0..9 / A..Z)        
        str_push(sstr,(wchar_t)wParam);
    }
    
    //_PRINTF("SSTR: %s (pos=%i)\n",sstr,pos);
    hwnd_InputUpdate(1);
    
    return 0;
}

void hwnd_CopyFromClipboard(void) {
    HANDLE hClipboardData;
    wchar_t *lpstr;
    int i;
    if (OpenClipboard(NULL)) {
        hClipboardData = GetClipboardData(CF_UNICODETEXT);
        lpstr = (wchar_t*)GlobalLock(hClipboardData);
        if (wcslen(lpstr) < 64) {
            wcscpy(sstr,lpstr);
        } else {
            for (i=0;i!=63;i++) {
                sstr[i] = lpstr[i];
            }
            sstr[i] = L'\0';
        }
        GlobalUnlock(hClipboardData);
        CloseClipboard();
    }
    hwnd_InputUpdate(1);
}

LRESULT CALLBACK WindowProcedure (HWND hwndW, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    RECT rect; 
    PAINTSTRUCT psPaint;
    HFONT hFont;
    
    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 12345:
#warning Ugly implementation of CTRL+V handling with accelerators. To be reworked...
                    hwnd_CopyFromClipboard();
                    break;
            }
            break;
        case WM_DESTROY:
            // Kill em all
            //PostQuitMessage (0); 
            _PRINTF(L"WM_DESTROY\n");
            if (erg != NULL) {
                erg_Delete(erg);                
                erg_pos = 0;
                erg = NULL;
            }
            break;
        case WM_KILLFOCUS:
            ShowWindow(hwnd,SW_HIDE);
            break;
        case WM_HOTKEY:
            Hotkey_Process(hwndW,wParam);
            break;
        case WM_SYSKEYDOWN:
            //printf("SYSKEYDOWN(wParam=%i,%c,%c\n",wParam,((lParam & (1<<29))?'1':'0'),((lParam & (1<<29))?'1':'0'));
            if ((lParam & (1<<30)) == 0) {
                if (wParam == VK_MENU) {
                    _PRINTF(L"ALT down\n");
                    // Show ConfigMenu
                    hwnd_ConfigShow();
                } else {
                    _PRINTF(L"ALT+%i\n",wParam);
                    // ProcessInputCombo
                    ProcessInputCombo(wParam);
                }
            }
            break;
        case WM_SYSKEYUP:
            if (wParam == VK_MENU) {
                _PRINTF(L"ALT up\n");
                // Hide ConfigMenu
                hwnd_ConfigHide();
            } 
            break;
        case WM_KEYDOWN:
            break;
        case WM_CHAR:
//            _PRINTF("WM_KEY: %i %c\n",wParam,wParam);
            ProcessInput(wParam);
            break;
        case WM_KEYUP:
            if (wParam == VK_MENU) {
                _PRINTF(L"ALT up (KEYUP)\n");
                // Hide ConfigMenu
                hwnd_ConfigHide();
            } else if (wParam == VK_HOME) { // POS1
                erg_pos = 0;
                hwnd_ResultShow(erg);
            } else if (wParam == VK_PRIOR) { // BILDHOCH
                if (erg_pos > 0)
                    erg_pos--;
                hwnd_ResultShow(erg);
            } else if (wParam == VK_NEXT) { // BILDRUNTER
                erg_pos++;
                hwnd_ResultShow(erg);
            }
            break;    
        default:                
            return DefWindowProc (hwndW, message, wParam, lParam);
    }

    return 0;
}


HWND hwnd_Init(void) {
    WNDCLASSEXW wincl;
    t_cfgmain *cfgm;
    
    // load configuration
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // window class
    wincl.hInstance = GetModuleHandle(NULL);
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure; // window handler
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(1483));
    wincl.hIconSm = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(1483));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    // background color
    wincl.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b));//GetStockObject(BLACK_BRUSH);    
    // reg wndclass
    if (!RegisterClassEx (&wincl)) {
        MessageBoxW(NULL, L"Failed to register window class, quitting...", NULL, MB_OK | MB_ICONERROR);
        return 0;
    }
    
    // create main window
    hwnd = CreateWindowExW (
           0,
           szClassName, // classname
           CONFIG_WINDOWTITLE, // hwnd title
           WS_POPUP|WS_SYSMENU, // type
           CW_USEDEFAULT, // position
           CW_USEDEFAULT, // size
           544, // width
           375, // height
           HWND_DESKTOP, // child to desktop
           NULL, // menu
           GetModuleHandle(NULL), // hinst
           NULL // creation data
           );      

    // window transparency
    SetWindowLong(hwnd, GWL_EXSTYLE,GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);        
    SetLayeredWindowAttributes(hwnd, 0, cfgm->hwnd_alpha, LWA_ALPHA);
    
    // clear search / initialize
    erg = NULL;
    erg_pos = 0;
    
    return hwnd;
}

void hwnd_Toggle(void) {
	RECT r;

	if (IsWindowVisible(hwnd)) {
		ShowWindow(hwnd,SW_HIDE);        
	} else {
		// window width/height --> max area
		SystemParametersInfo(SPI_GETWORKAREA,0,&r,0);
		// window position
		SetWindowPos(hwnd,HWND_TOPMOST,0,0,r.right,r.bottom,SWP_NOACTIVATE|SWP_SHOWWINDOW);

		ShowWindow(hwnd,SW_SHOW);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);	

        // refresh input area
        hwnd_InputUpdate(1);
        
        // show results if available
        if (erg != NULL) {
            hwnd_ResultShow(erg);
        }
	}
}

void hdc_MakeTransparent(HDC hdc,int dx,int dy) {
    int i,o;
    COLORREF coltrans = GetPixel(hdc,0,0);
    COLORREF colbg;
    t_cfgmain *cfgm;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    colbg = RGB(cfgm->hwnd_r,cfgm->hwnd_g,cfgm->hwnd_b);
    
    for (i=0;i!=dx;i++) {
        for (o=0;o!=dy;o++) {
            if (coltrans == GetPixel(hdc,i,o)) {
                SetPixel(hdc,i,o,colbg);
            }
        }
    }
}

void hwnd_LoadBitmap(wchar_t *bmp_filename) {
    t_cfgmain *cfgm;
    HDC hdchwnd;
    BITMAP bm;    
    
    // load cfg
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    hdchwnd = GetDC(hwnd);
    // release memory
    if (cfgm->hdcbmp != NULL) {
        DeleteDC(cfgm->hdcbmp);
        cfgm->hdcbmp = NULL;
    }
    if (cfgm->hbmp != NULL) {
        DeleteObject(cfgm->hbmp);
        cfgm->hbmp = NULL;
    }
    
    cfgm->hbmp = LoadImage(GetModuleHandle(NULL),bmp_filename,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
    if (cfgm->hbmp == NULL) {
        _PRINTF(L"ERR: hwnd_LoadBitmap() - LoadImage(%s)\n",bmp_filename);
        return;
    }
    GetObject(cfgm->hbmp,sizeof(bm),&bm);
    cfgm->hdcbmp = CreateCompatibleDC(hdchwnd);
    SelectObject(cfgm->hdcbmp,cfgm->hbmp);
    hdc_MakeTransparent(cfgm->hdcbmp,bm.bmWidth,bm.bmHeight);
    
    ReleaseDC(hwnd,hdchwnd);    
}
