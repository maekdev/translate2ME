// cfg_file.c
//
// 240918
// markus ekler
//
// rewritten UNICODE version of cfg_file
// added support for compressed dictionaries (zip/miniz)

#include "config.h"
#include "hwnd.h"
#include "cfg_file.h"
#include "cfg_parser.h"

#include "miniz.h"

// typedef definition
typedef int(*lp_FUNCTION)(t_scanner *sc);
// chain of parameters 
typedef struct {
    wchar_t *str;
    int type;
    lp_FUNCTION f;
    int *ti;
    wchar_t *tstr;
} t_cfgchain;

// prototypes for callable functions
int FKT_SetHotkeyMod(t_scanner *sc);
int FKT_ConfigLang(t_scanner *sc);

int FKT_LoadDictionary(t_scanner *sc);
int FKT_LoadDictionaryZIP(t_scanner *sc);
int FKT_LoadBitmap(t_scanner *sc);

int FKT_DeleteComment(t_scanner *sc);
int FKT_DeleteString(t_scanner *sc);
int FKT_ReplaceChar(t_scanner *sc);
int FKT_RegexSubstitute(t_scanner *sc);


// local structure for global configuration
t_cfgmain cfgmain;



// cfg_main config tokens & links
t_cfgchain maincfgchain[] = { 
    // tokenstring, t_scanner->type, functionpointer, integer to assign(int), string to assign(str)
    L"RESULTSMAX",T_ZUWEISUNGINT,NULL,&(cfgmain.resultsmax),NULL,
    L"RESULTSMIN",T_ZUWEISUNGINT,NULL,&(cfgmain.resultsmin),NULL,
    L"HOTKEY_MOD",T_FUNK1,FKT_SetHotkeyMod,NULL,NULL,
    L"HOTKEY_ID",T_ZUWEISUNGINT,NULL,&(cfgmain.hotkey_id),NULL,
    L"BGCOL_R",T_ZUWEISUNGINT,NULL,&(cfgmain.hwnd_r),NULL,
    L"BGCOL_G",T_ZUWEISUNGINT,NULL,&(cfgmain.hwnd_g),NULL,
    L"BGCOL_B",T_ZUWEISUNGINT,NULL,&(cfgmain.hwnd_b),NULL,
    L"FGCOL_R",T_ZUWEISUNGINT,NULL,&(cfgmain.font_r),NULL,
    L"FGCOL_G",T_ZUWEISUNGINT,NULL,&(cfgmain.font_g),NULL,
    L"FGCOL_B",T_ZUWEISUNGINT,NULL,&(cfgmain.font_b),NULL,
    L"HLCOL_R",T_ZUWEISUNGINT,NULL,&(cfgmain.fonthl_r),NULL,
    L"HLCOL_G",T_ZUWEISUNGINT,NULL,&(cfgmain.fonthl_g),NULL,
    L"HLCOL_B",T_ZUWEISUNGINT,NULL,&(cfgmain.fonthl_b),NULL,
    L"INPUT_FONTSIZE",T_ZUWEISUNGINT,NULL,&(cfgmain.inputfontsize),NULL,
    L"TEXT_FONTSIZE",T_ZUWEISUNGINT,NULL,&(cfgmain.textfontsize),NULL,
    L"WNDALPHA",T_ZUWEISUNGINT,NULL,&(cfgmain.hwnd_alpha),NULL,
    L"INPUT_HEIGHT",T_ZUWEISUNGINT,NULL,&(cfgmain.input_height),NULL,
    L"LANG_CFG",T_FUNK2,FKT_ConfigLang,NULL,NULL,
    // veraenderliche optionen
    STR_LANGLAST,T_ZUWEISUNGINT,NULL,&(cfgmain.lang_last),NULL,
    STR_SEARCHSUBST,T_ZUWEISUNGINT,NULL,&(cfgmain.search_subst),NULL,    
    STR_SEARCHIGNORECASE,T_ZUWEISUNGINT,NULL,&(cfgmain.search_ignorecase),NULL,
    STR_SEARCHSEARCH,T_ZUWEISUNGINT,NULL,&(cfgmain.search_search),NULL,
    STR_SEARCHAPPROX,T_ZUWEISUNGINT,NULL,&(cfgmain.search_approx),NULL,
    NULL,0,NULL,NULL,NULL };

// cfg chain specific for language files
t_cfgchain langcfgchain[] = {
    // tokenstring, t_scanner->type, function pointer, integer to assign(int), string to assign(str)
    L"LOADDICTIONARY",T_FUNK1,FKT_LoadDictionary,NULL,NULL,
	L"LOADDICTIONARYZIP",T_FUNK1,FKT_LoadDictionaryZIP,NULL,NULL,
    L"LOADBITMAP",T_FUNK1,FKT_LoadBitmap,NULL,NULL,
    L"SEARCHURL",T_ZUWEISUNGSTR,NULL,NULL,cfgmain.searchurl,
    L"DELETECOMMENT",T_FUNK0,FKT_DeleteComment,NULL,NULL,
    L"DELETESTRING",T_FUNK1,FKT_DeleteString,NULL,NULL,
    L"REPLACECHAR",T_FUNK2,FKT_ReplaceChar,NULL,NULL,
    L"SUBSTITUTE",T_FUNK2,FKT_RegexSubstitute,NULL,NULL,
    NULL,0,NULL,NULL,NULL };
    
int cfgmain_Init(void) {
    int i;
	// default values
    wcscpy(cfgmain.searchurl,L"");
    cfgmain.data = NULL;
    cfgmain.datalen = 0;
    cfgmain.rschain = NULL;
    cfgmain.hdcbmp = NULL;
    cfgmain.hbmp = NULL;
    cfgmain.resultsmax = 500;
    cfgmain.resultsmin = 5;
    cfgmain.hotkey_mod = MOD_WIN|MOD_CONTROL;
    cfgmain.hotkey_id = 0x4A;
    cfgmain.hwnd_r = 0;
    cfgmain.hwnd_g = 0;
    cfgmain.hwnd_b = 0;
    cfgmain.font_r = 255;
    cfgmain.font_g = 255;
    cfgmain.font_b = 255;
    cfgmain.fonthl_r = 255;
    cfgmain.fonthl_g = 255;
    cfgmain.fonthl_b = 255;
    cfgmain.inputfontsize = 80;
    cfgmain.textfontsize = 40;
    cfgmain.hwnd_alpha = 200;
    cfgmain.input_height = 100;
    for (i=0;i!=9;i++) {
        cfgmain.lang_cfgfile[i] = NULL;
        cfgmain.lang_cfgname[i] = NULL;
    }
    // default values for changeable values / lastconfig
    cfgmain.lang_last = 0;
    cfgmain.search_subst = 1;
    cfgmain.search_ignorecase = 1;
    cfgmain.search_search = 2; // default: wildcard search
    cfgmain.search_approx = 1;
	return 0;
}

int cfgmain_Destroy(void) {
    int i;
    FILE *f;
    wchar_t cfgfn[_MAX_PATH];
    
    // Dateierstellen mit den letzten Benutzerkonfigurationen und vorher allokierte Speicherbereiche freigeben
    
    // dictionary release
    if (cfgmain.data != NULL) {
        free(cfgmain.data);
        cfgmain.data = NULL;
    }
    // regex subst release
    if (cfgmain.rschain != NULL) {
        rschain_Destroy();
    }
    // graphics release
    if (cfgmain.hdcbmp != NULL) {
        DeleteDC(cfgmain.hdcbmp);
    }
    if (cfgmain.hbmp != NULL) {
        DeleteObject(cfgmain.hbmp);
    }
    
    // free language configuration
    for (i=0;i!=9;i++) {
        if (cfgmain.lang_cfgfile[i] != NULL) {
            free(cfgmain.lang_cfgfile[i]);
            cfgmain.lang_cfgfile[i] = NULL;
        }
        if (cfgmain.lang_cfgname[i] != NULL) {
            free(cfgmain.lang_cfgfile[i]);
            cfgmain.lang_cfgname[i] = NULL;
        }
    }
    
    // save config file with last configuration options
    GetModuleFileName(NULL,cfgfn,_MAX_PATH);
    for (i=wcslen(cfgfn);i--;i>0) {
        if (cfgfn[i] == '\\')
            break;            
    }
    cfgfn[i+1] = '\0';
    wcscat(cfgfn,FILENAME_CFGLAST);
    
    f = _wfopen(cfgfn,L"w");
    if (f != NULL) {
        fwprintf(f,L"# Automatically generated file. Do not edit!\n");
        fwprintf(f,L"%ls=%i\n",STR_LANGLAST,cfgmain.lang_last);
        fwprintf(f,L"%ls=%i\n",STR_SEARCHSUBST,cfgmain.search_subst);
        fwprintf(f,L"%ls=%i\n",STR_SEARCHIGNORECASE,cfgmain.search_ignorecase);
        fwprintf(f,L"%ls=%i\n",STR_SEARCHSEARCH,cfgmain.search_search);
        fwprintf(f,L"%ls=%i\n",STR_SEARCHAPPROX,cfgmain.search_approx);
        fwprintf(f,L"# end of file (%ls)\n",cfgfn);
        fclose(f);
    }
}

t_cfgmain* cfgmain_Get(void) {
    return (&cfgmain);
}

int cfgmain_Process(t_scanner *sc) {
    int i;
    
    if (sc->type == T_FEHLER) {
        _PRINTF(L"WARNING: cfg syntax error (%ls)\n",sc->id);
        return 1;
    }
    
    for (i=0;maincfgchain[i].str != NULL;i++) {
        if (wcscmp(sc->id,maincfgchain[i].str) == 0) {
            if (sc->type == maincfgchain[i].type) {
                _PRINTF(L" -> %ls, ",sc->id);
                switch (maincfgchain[i].type) {
                    case T_ZUWEISUNGINT:                        
                        if (maincfgchain[i].ti == NULL) {
                            _PRINTF(L"ERROR (NULL)\n");
                            return 1;
                        }
                        _PRINTF(L"SET INT (%i)\n",sc->i);
                        *(maincfgchain[i].ti) = sc->i;
                        return 0;
                    case T_ZUWEISUNGSTR:
                        if (maincfgchain[i].tstr == NULL) {
                            _PRINTF(L"ERROR (NULLSTR)\n");
                            return 1;
                        }
                        _PRINTF(L"SET STR (%ls)\n",sc->str1);
                        wcscpy(maincfgchain[i].tstr,sc->str1);
                        break;
                    case T_FUNK0:
                    case T_FUNK1:
                    case T_FUNK2:
                        if (maincfgchain[i].f == NULL) {
                            _PRINTF(L"ERROR (NULL FCT)\n");
                            return 1;
                        }
                        _PRINTF(L"EXEC\n");
                        maincfgchain[i].f(sc);
                        return 0;
                }
            }
        }
    }
    return 0;
}

wchar_t *_wReadFileUTF8(wchar_t* fn,int *l) {
	// small helper function to unify the steps needed to read & convert a UTF8 file
	// returns NULL in case of fail, else buffer with file content & bufferlen - l
	// REMEMBER: release buffer (free()) when no longer needed.
	FILE *f;
	char *cbuf;
	wchar_t *wbuf;
	int flen;
	
	f = _wfopen(fn,L"rb");
	if (f==NULL) {
			*l = -1;
			return NULL;
	}
	
	fseek(f,0,SEEK_END);
	flen = ftell(f);
	rewind(f);
	
	cbuf = (char*)malloc(flen+1);
	if (cbuf == NULL) {
		*l = -2;
		return NULL;
	}
	fread(cbuf,1,flen,f);
	fclose(f);
	
	*l = MultiByteToWideChar(CP_UTF8,0,cbuf,flen,NULL,0);
	if (*l == 0) {
		*l = -3;
		return NULL;
	}
	wbuf = (wchar_t*)malloc(*l*sizeof(wchar_t));
	if (wbuf == NULL) {
		*l = -4;
		return NULL;
	}
	if (*l != MultiByteToWideChar(CP_UTF8,0,cbuf,flen,wbuf,*l)) {
		*l = -5;
		return NULL;
	}
	
	free(cbuf);
	
	return wbuf;
}

int cfgmain_ReadFile(void) {
    wchar_t cfgfn[_MAX_PATH];
	wchar_t *wbuf;
	int flen;
	int i;
	t_scanner sc;

    // set filename relative to executable path
    GetModuleFileNameW(NULL,cfgfn,_MAX_PATH);
    for (i=wcslen(cfgfn);i--;i>0) {
        if (cfgfn[i] == '\\')
            break;            
    }
    cfgfn[i+1] = '\0';
    wsprintf(cfgfn,L"%ls%ls",cfgfn,FILENAME_CFGMAIN);
    
    // read main cfg file
	wbuf = _wReadFileUTF8(cfgfn,&flen);
	if (wbuf == NULL) {
			_PRINTF(L"cfgmain_ReadFile(): config file failed.\n");
			return 1;
	}

    // process main cfg file
	i = 0;
    while (i<flen) {
        i += cfg_scanner(&sc,wbuf+i,flen-i);
        cfgmain_Process(&sc);
    }

	// free main cfg buffer
    free(wbuf);
    
    // set filepath relative to executable
    GetModuleFileNameW(NULL,cfgfn,_MAX_PATH);
    for (i=wcslen(cfgfn);i--;i>0) {
        if (cfgfn[i] == '\\')
            break;            
    }
    cfgfn[i+1] = '\0';
    wcscat(cfgfn,FILENAME_CFGLAST);
    
	// read last cfg file into buffer
    wbuf = _wReadFileUTF8(cfgfn,&flen);
	if (wbuf == NULL) {
		_PRINTF(L"cfgmain_ReadFile(): last config file failed.\n");
		return 1;
	}
    
    // process last cfg file
	i = 0;
    while (i<flen) {
        i += cfg_scanner(&sc,wbuf+i,flen-i);
        cfgmain_Process(&sc);
    }
    
	free(wbuf);
	return 0;
}

int cfglang_Process(t_scanner *sc) {
    int i;
    
    if (sc->type == T_FEHLER) {
        _PRINTF(L"WARNING: cfg syntax error (%ls)\n",sc->id);
        return 1;
    }
    
    for (i=0;langcfgchain[i].str != NULL;i++) {
        if (wcscmp(sc->id,langcfgchain[i].str) == 0) {
            if (sc->type == langcfgchain[i].type) {
                _PRINTF(L" -> %ls, ",sc->id);
                switch (langcfgchain[i].type) {
                    case T_ZUWEISUNGINT:                        
                        if (langcfgchain[i].ti == NULL) {
                            _PRINTF(L"ERROR (NULL)\n");
                            return 1;
                        }
                        _PRINTF(L"SET INT (%i)\n",sc->i);
                        *(langcfgchain[i].ti) = sc->i;
                        return 0;
                    case T_ZUWEISUNGSTR:
                        if (langcfgchain[i].tstr == NULL) {
                            _PRINTF(L"ERROR (NULLSTR)\n");
                            return 1;
                        }
                        _PRINTF(L"SET STR (%ls)\n",sc->str1);
                        wcscpy(langcfgchain[i].tstr,sc->str1);
                        break;
                    case T_FUNK0:
                    case T_FUNK1:
                    case T_FUNK2:
                        if (langcfgchain[i].f == NULL) {
                            _PRINTF(L"ERROR (NULLFKT)\n");
                            return 1;
                        }
                        _PRINTF(L"EXEC\n");
                        langcfgchain[i].f(sc);
                        return 0;
                }
            }
        }
    }
    return 0;
}

int cfglang_ReadFile(wchar_t *langfn) {
    wchar_t cfgfn[_MAX_PATH];
	wchar_t *wbuf;
	int flen;
	int i;
	t_scanner sc;

    // set filename in relation to executable position
    GetModuleFileNameW(NULL,cfgfn,_MAX_PATH);
    for (i=wcslen(cfgfn);i--;i>0) {
        if (cfgfn[i] == '\\')
            break;            
    }
    cfgfn[i+1] = '\0';
    wsprintf(cfgfn,L"%ls%ls",cfgfn,langfn);
    
    // read language file into buffer
	wbuf = _wReadFileUTF8(cfgfn,&flen);
	if (wbuf == NULL) {
		_PRINTF(L"cfglan_ReadFile(): file i/o fail.\n");
		return 1;
	}

	// before processing the language specific substitution is set to default
    rschain_Destroy();
    
    // interpret language file
	i = 0;
    while (i<flen) {
        i += cfg_scanner(&sc,wbuf+i,flen-i);
		_PRINTF(L"%i",i);
        cfglang_Process(&sc);
    }

	free(wbuf);
	return 0;
}

int FKT_SetHotkeyMod(t_scanner *sc) {
    int i=0;
    wchar_t *s = sc->str1;
    
    while (*s) {
        switch (*s) {
            case 'w':
            case 'W': i |= MOD_WIN; break;
            case 'a':
            case 'A': i |= MOD_ALT; break;
            case 'c': 
            case 'C': i |= MOD_CONTROL; break;            
        }
        s++;
    }
    
    if (i != 0) {
        cfgmain.hotkey_mod = i;
    }
    
    return 0;
}

int FKT_ConfigLang(t_scanner *sc) {
	int i=0;
    while (i<9) {
        if (cfgmain.lang_cfgname[i] == NULL) {
            cfgmain.lang_cfgname[i] = (wchar_t*)malloc((wcslen(sc->str1)+2)*sizeof(wchar_t));;
            if (cfgmain.lang_cfgname[i] == NULL) {
                _PRINTF(L"ERR: FKT_ConfigLang() - malloc()\n");
                return 1;
            }
            wcscpy(cfgmain.lang_cfgname[i],sc->str1);
            cfgmain.lang_cfgfile[i] = (wchar_t*)malloc((wcslen(sc->str2)+2)*sizeof(wchar_t));
            if (cfgmain.lang_cfgfile[i] == NULL) {
                _PRINTF(L"ERR: FKT_ConfigLang() - malloc()\n");
                return 2;
            }
            wcscpy(cfgmain.lang_cfgfile[i],sc->str2);
            _PRINTF(L"    FKT_ConfigLang(): (%i) = %ls \n",i,sc->str1);
            return 0;
        }
        i++;
    }
    _PRINTF(L"ERR: FKT_ConfigLang() - Too many language configurations (%ls)\n",sc->str1);
    return 0;
}

int FKT_LoadDictionary(t_scanner *sc) {
	wchar_t dictfn[_MAX_PATH];
    FILE *f;
    int i;
    
    // filename
    GetModuleFileNameW(NULL,dictfn,_MAX_PATH);
    for (i=wcslen(dictfn);i--;i>0) {
        if (dictfn[i] == L'\\')
            break;            
    }
    dictfn[i+1] = L'\0';
    wcscat(dictfn,sc->str1);
    
	if (cfgmain.data != NULL) {
		free(cfgmain.data);
	}
	cfgmain.data = _wReadFileUTF8(dictfn,&(cfgmain.datalen));
	if (cfgmain.data == NULL) {
		_PRINTF(L"LoadDictionary(): file read error (%ls)\n",dictfn);
		return 1;
	}
	
    // normalise ( [\r\n...] = \0
    for (i=0;i!=cfgmain.datalen;i++) {
        if (cfgmain.data[i] == '\n' || cfgmain.data[i] == '\r') {
            cfgmain.data[i] = '\0';
        }
    }
    
    _PRINTF(L"LoadDictionary(): %ls - %i bytes read\n",sc->str1,cfgmain.datalen);
    return 0;
}

int FKT_LoadDictionaryZIP(t_scanner *sc) {
	wchar_t dictfn[_MAX_PATH];
	char dictfn8[_MAX_PATH]; // miniz does not support wchar_t so convert to ansi
    FILE *f;
    int i;
	
	// zip archive stuff
	mz_zip_archive zip_archive;
	memset(&zip_archive,0,sizeof(zip_archive));
    
    // filename
    GetModuleFileNameW(NULL,dictfn,_MAX_PATH);
    for (i=wcslen(dictfn);i--;i>0) {
        if (dictfn[i] == L'\\')
            break;            
    }
    dictfn[i+1] = L'\0';
    wcscat(dictfn,sc->str1);
    
	if (cfgmain.data != NULL) {
		free(cfgmain.data);
	}
	/*cfgmain.data = _wReadFileUTF8(dictfn,&(cfgmain.datalen));
	if (cfgmain.data == NULL) {
		_PRINTF(L"LoadDictionary(): file read error (%ls)\n",dictfn);
		return 1;
	}*/
	
	uint32_t size_needed = WideCharToMultiByte(CP_UTF8,0,dictfn,-1,NULL,0,NULL,NULL);
	WideCharToMultiByte(CP_UTF8,0,dictfn,-1,dictfn8,size_needed,NULL,NULL);
	
	// open zip file
	if (!mz_zip_reader_init_file(&zip_archive,dictfn8,0)) {
		_PRINTF(L"LoadDictionaryZIP(): zip file init error (%ls)\n",dictfn);
		return 1;
	}
	
	// check if min. one file is in the archive and get filename
	int num_files = mz_zip_reader_get_num_files(&zip_archive);
	if (num_files < 1) {
		_PRINTF(L"LoadDictionaryZIP(): zip file error - no files found (%i)\n",num_files);
		mz_zip_reader_end(&zip_archive);
		return 2;
	}
	char filename[_MAX_PATH];
	if (!mz_zip_reader_get_filename(&zip_archive,0,filename,_MAX_PATH)) {
		_PRINTF(L"LoadDictionaryZIP(): zip file error - unable to get filename\n");
		return 3;
	}
	_PRINTF(L"LoadDictionaryZIP(): Found filename [%s]\n",filename);
	
	// open filename handle, allocate memory and load into memory
	mz_zip_archive_file_stat file_stat;
	if (!mz_zip_reader_file_stat(&zip_archive,0,&file_stat)) {
		_PRINTF(L"LoadDictionaryZIP(): zip error - file_stat failed.\n");
		mz_zip_reader_end(&zip_archive);
		return 4;
	}
	
	char *tmp = NULL;
	tmp = (char*)malloc(file_stat.m_uncomp_size+1);
	if (tmp == NULL) {
		_PRINTF(L"LoadDictionaryZIP(): malloc tmp failed.\n");
		mz_zip_reader_end(&zip_archive);
		return 5;
	}
	cfgmain.datalen = MultiByteToWideChar(CP_UTF8,0,tmp,file_stat.m_uncomp_size,NULL,0);
	if (cfgmain.datalen == 0) {
		_PRINTF(L"LoadDictionaryZIP(): invalid WideChar conversion size\n");
		free(tmp);
		mz_zip_reader_end(&zip_archive);
		return 6;
	}
	cfgmain.data = (wchar_t*)malloc(cfgmain.datalen*sizeof(wchar_t)+1);
	if (cfgmain.data == NULL) {
		_PRINTF(L"LoadDictionaryZIP(): malloc maindata failed.\n");
		free(tmp);
		mz_zip_reader_end(&zip_archive);
		return 7;
	}
	if (!mz_zip_reader_extract_to_mem(&zip_archive,0,tmp,file_stat.m_uncomp_size,0)) {
		_PRINTF(L"LoadDictionaryZIP(): error uncompressing zip file (%s / %i)\n",filename,cfgmain.datalen);
		free(tmp);
		mz_zip_reader_end(&zip_archive);
		return 8;
	}	
	
	// size_needed can be smaller than uncompressed byte length, because stored in wchar_t
	size_needed = MultiByteToWideChar(CP_UTF8,0,tmp,file_stat.m_uncomp_size,cfgmain.data,cfgmain.datalen);	              
	_PRINTF(L"wchar conversion: realloc buffer for %i wchar (%i released)\n",size_needed,cfgmain.datalen-size_needed);
	cfgmain.datalen = size_needed;
	cfgmain.data = (wchar_t*)realloc(cfgmain.data,(size_needed+1)*sizeof(wchar_t));
	
	_PRINTF(L"Memory allocated: %i Bytes\nFile in Zip: %i Bytes\n\nConverted: %i Bytes",file_stat.m_uncomp_size+1,cfgmain.datalen,size_needed);
	
	
	free(tmp);
		
	// close zip file
	mz_zip_reader_end(&zip_archive);
	
    // normalise ( [\r\n...] = \0
    for (i=0;i!=cfgmain.datalen;i++) {
        if (cfgmain.data[i] == '\n' || cfgmain.data[i] == '\r') {
            cfgmain.data[i] = '\0';
        }
    }
    
    _PRINTF(L"LoadDictionaryZIP(): %ls - %i bytes read\n",sc->str1,cfgmain.datalen);
    return 0;
}


int FKT_LoadBitmap(t_scanner *sc) {
    wchar_t bmpfn[_MAX_PATH];
    int i;
    
    // filename
    GetModuleFileNameW(NULL,bmpfn,_MAX_PATH);
    for (i=wcslen(bmpfn);i--;i>0) {
        if (bmpfn[i] == '\\')
            break;            
    }
    bmpfn[i+1] = '\0';
    wcscat(bmpfn,sc->str1);
    
    hwnd_LoadBitmap(bmpfn);
    return 0;
}

int FKT_DeleteComment(t_scanner *sc) {
    int i,o;
    o=0;
    for (i=0;i<cfgmain.datalen;i++) {
        if (*(cfgmain.data+i) == '#') {
            while (i<cfgmain.datalen && *(cfgmain.data+i) != '\0')
                i++;
        } else {
            *(cfgmain.data+o) = *(cfgmain.data+i);
            o++;
        }
    }
    // reduce memory
    _PRINTF(L"DeleteComment(): %i Bytes freigegeben.\n",cfgmain.datalen-o);
    cfgmain.data = (wchar_t*)realloc(cfgmain.data,(o+1)*sizeof(wchar_t));
    cfgmain.datalen = o;
            
    return 0;
}

int mestrcmp(wchar_t *instr,wchar_t *pat) {
    int i;    
    for (i=0;i!=wcslen(pat);i++) {
        if (*(instr+i) != *(pat+i)) {
            return 0;
        }
    }
    
    return 1;
}

int FKT_DeleteString(t_scanner *sc) {
    int i,o;
    
    if (wcslen(sc->str1) == 0) {
        _PRINTF(L"FKT_DeleteString(): malicious parameter.\n");
        return 1;
    }
    
    o=0;
    for (i=0;i<cfgmain.datalen-wcslen(sc->str1);i++) {
        if (mestrcmp((cfgmain.data)+i,sc->str1)) {
            i+=wcslen(sc->str1)-1;
        } else {
            *(cfgmain.data+o) = *(cfgmain.data+i);
            o++;
        }
    }
    
    // reduce memory
    _PRINTF(L"DeleteString(): %i bytes released.\n",cfgmain.datalen-o);
    cfgmain.data = (wchar_t*)realloc(cfgmain.data,(o+1)*sizeof(wchar_t));
    cfgmain.datalen = o;
    
    return 0;
}

int FKT_ReplaceChar(t_scanner *sc) {
    int i;
    int o=0;
    
    if (wcslen(sc->str1) == 0 || wcslen(sc->str2) == 0) {
        _PRINTF(L"FKT_ReplaceChar(): malicious parameter.\n");
        return 1;
    }
    
    for (i=0;i<cfgmain.datalen;i++) {
        if (*(cfgmain.data+i) == *(sc->str1)) {
            *(cfgmain.data+i) = *(sc->str2);
            o++;
        }
    }
    
    _PRINTF(L"ReplaceChar(): %i occurances changed (%ls)\n",o,sc->str2);
    
    return 0;
}

void rschain_Destroy(void) {
    // releases the regexsubst chain
    t_regexsubst *rs;
    t_regexsubst *rs_tmp;
    
    rs = cfgmain.rschain;
    while (rs != NULL) {
        rs_tmp = rs->next;
        free(rs->s);        
        free(rs);   
        rs = rs_tmp;
    }
    cfgmain.rschain = NULL;
}

void rschain_Append(wchar_t c,wchar_t *str) {
	// search in the current chain for a character and extend entry
	// if not available, extend the chain
    t_regexsubst *rs;
    wchar_t *pstr;
    
    rs = cfgmain.rschain;
    while (rs != NULL) {
        if (c==rs->c) {
            // entry found
            pstr = (wchar_t*)malloc((wcslen(str)+wcslen(rs->s)+1)*sizeof(wchar_t));
            wcscpy(pstr,rs->s);
            wcscat(pstr,str);
            free(rs->s);
            rs->s = pstr;
            return;
        }
        rs = rs->next;
    }
    // entry not found
    //  -> create entry and add to the beginning
    rs = (t_regexsubst*)malloc(sizeof(t_regexsubst));
    if (rs != NULL) {
        rs->c = c;
        rs->next = cfgmain.rschain;
        rs->s = (wchar_t*)malloc((wcslen(str)+1)*sizeof(wchar_t));
        if (rs->s != NULL) {
            wcscpy(rs->s,str);
        }
        cfgmain.rschain = rs;
    }
}

wchar_t *rschain_Find(wchar_t c) {
	// returns a pointer to the string of the search character
    wchar_t *ret;
    t_regexsubst *rs;
    
    rs = cfgmain.rschain;
    ret = NULL;
    while (rs != NULL) {
        if (c==rs->c) {
            return (rs->s);
        }
        rs = rs->next;
    }
    return ret;
}

int FKT_RegexSubstitute(t_scanner *sc) {
    if (wcslen(sc->str1) == 0 || wcslen(sc->str2) == 0) {
        _PRINTF(L"RegexSubstitute(): malicious parameter.\n");
        return 1;
    }
    rschain_Append(*(sc->str1),sc->str2);
    _PRINTF(L"RegexSubstitute(): %ls -> %ls\n",sc->str1,sc->str2);
    return 0;
}