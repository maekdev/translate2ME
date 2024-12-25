#ifndef __SEARCH_H
#define __SEARCH_H


typedef struct t_ergebnis t_ergebnis;
struct t_ergebnis {
    int abs_start;
    int offset_start;
    int offset_stop;
    int rate;
    t_ergebnis *next;
};

int search_Create(wchar_t *inreg);
int search_Destroy(void);
wchar_t* search_ErrDecode(int errcode);
t_ergebnis *search_Search(void);

int search_Count(t_ergebnis *erg);
void search_Sort(t_ergebnis *ergebnisse);

// Bugfix: erg_Delete used in hwnd.c
void erg_Delete(t_ergebnis *erg);

#endif // __SEARCH_H
