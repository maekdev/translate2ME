#ifndef __CONFIG_H
#define __CONFIG_H

//// CONFIG

#ifdef __GNUC__
	#define STRINGIFY(x) #x
	#define TO_STRING(x) STRINGIFY(x)
	#define COMPILER_VERSION L"GCC " TO_STRING(__GNUC__) L"." TO_STRING(__GNUC_MINOR__)
#endif
/* include here missing compiler identifer strings */
#ifndef COMPILER_VERSION
	#define COMPILER_VERSION L"(unknown compiler)"
#endif

#define INFO_STRING 		L"translateME 2.0alpha (created by Markus Ekler built on " __DATE__ " " __TIME__ " using " COMPILER_VERSION ")"
// mutex identifier (keep same over all versions)
#define MUTEXNAME           L"DrWortklauber"
// main window title
#define CONFIG_WINDOWTITLE	L"DrWortklauber2 (Offline Dictionary translate2ME)"
// konfigurationsdatei
#define FILENAME_CFGMAIN	L"config.cfg"
// letzte veraenderlichen einstellungen
#define FILENAME_CFGLAST    L"last.cfg"
// regex search string length
#define SSTRLEN     1024
// search history length (jede history eintrag wird als bitmap gespeichert)
#define HISTLEN     10
// breite des sprach bildes (rechts oben)
#define IMAGE_WIDTH     360

#define STR_LANGLAST            L"LANG_LAST"
#define STR_SEARCHSUBST         L"SEARCH_SUBST"
#define STR_SEARCHIGNORECASE    L"SEARCH_IGNORECASE"
#define STR_SEARCHSEARCH        L"SEARCH_SEARCH"
#define STR_SEARCHAPPROX        L"SEARCH_APPROX"

//// INCLUDE 
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <stdio.h>

//// STRUCT
typedef struct t_regexsubst t_regexsubst;
struct t_regexsubst {
    wchar_t c;     // search character
    wchar_t *s;    // substitution string
    t_regexsubst *next;
};

typedef struct {  
    // private data
    wchar_t searchurl[1024];
    wchar_t *data;
    int datalen;
    t_regexsubst *rschain;
    HDC hdcbmp;
    HBITMAP hbmp;
    // config datei
    int resultsmax;
    int resultsmin;
    int hotkey_mod;
    int hotkey_id;
    int hwnd_r;
    int hwnd_g;
    int hwnd_b;
    int font_r;
    int font_g;
    int font_b;
    int fonthl_r;
    int fonthl_g;
    int fonthl_b;
    int inputfontsize;
    int textfontsize;
    int hwnd_alpha;
    int input_height;
    wchar_t *lang_cfgfile[9];
    wchar_t *lang_cfgname[9];
    // veraenderliche Einstellungen
    int lang_last;          // index der zuletzt gewaehlten sprache
    int search_subst;       // sprachspezifische substitution an
    int search_ignorecase;  // gross/klein schreibung ignorieren
    int search_search;      // 0=literal / 1=regex / 2=wildcard
    int search_approx;      // unscharfe suche aktivieren
} t_cfgmain;

// definition for type in scanner_t
enum { T_FEHLER=16,T_ZUWEISUNGINT,T_ZUWEISUNGSTR,T_FUNK0,T_FUNK1,T_FUNK2,T_DEFINITION };

//typedef scanner_t scanner_t;
typedef struct {
    int type;
    wchar_t id[256];
    wchar_t str1[512];
    wchar_t str2[512];
    int i;
} t_scanner;               

//// PROTOTYPE
#ifdef _DEBUG
void _PRINTF(const wchar_t *,...);
#else
#define _PRINTF(...)
#endif

// refernce to main.c->DestroyApplication needed in hwnd.c
int DestroyApplication(void);

#endif // __CONFIG_H