#ifndef __CFG_FILE_H
#define __CFG_FILE_H

int cfgmain_Init(void);
int cfgmain_Destroy(void);
int cfgmain_ReadFile(void);
int cfglang_ReadFile(wchar_t *langfn);
t_cfgmain* cfgmain_Get(void);

wchar_t *rschain_Find(wchar_t c);
void rschain_Destroy(void);

#endif // __CFG_FILE_H
