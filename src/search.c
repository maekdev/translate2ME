// search.c
//
// 240918
// markus ekler
//
// rewritten version to support wchar_t
// sw layer to access trelib
//

#include "config.h"
#include "search.h"
#include "cfg_file.h"
#include "regex.h"


// local data for regex search
regex_t preg;
wchar_t inreg[SSTRLEN]; // local copy for trelib
wchar_t errmsg[1024];

// prototypes
void erg_Sort(t_ergebnis *ergebnisse);
//void erg_Delete(t_ergebnis *erg);
t_ergebnis* erg_Create(int abs_start,int abs_stop);
void erg_Append(t_ergebnis *chain,t_ergebnis *erg);


// functions
int search_Create(wchar_t *insstr) { 
	// REMARK: the input string inreg for trecomp is not allowed to change before the search process
    int ret;
    int flags=0;
    t_cfgmain *cfgm;
    
    wchar_t *s_find;
    int i,o;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // decode flags from cfg
    if (cfgm->search_search) { // REGEX, WILDCARDS
        for (i=0;i<SSTRLEN;i++) {
            inreg[i] = '\0';
        }
        for (i=0;i!=wcslen(insstr);i++) {
            // [] copy without change
            if (insstr[i] == '[') {
                do {
                    inreg[wcslen(inreg)] = insstr[i++];
                } while (*(insstr+i) != ']' && i<SSTRLEN-2);
                continue;
            }
            // wildcard handler
            if (cfgm->search_search == 2) { // wildcard
                if (insstr[i] == '*') {
                    wcscat(inreg,L"\\w*");
                    //i++;
                    continue;
                } else if (insstr[i] == '?') {
                    wcscat(inreg,L"\\w");
                    //i++;
                    continue;
                }
            }
            // substitution
            s_find = (wchar_t*)rschain_Find(insstr[i]);
            if (s_find) {
                wcscat(inreg,s_find);
                continue;
            }
            inreg[wcslen(inreg)] = insstr[i];
        }
    } else { // LITERAL
        wcscpy(inreg,insstr);
    }
    
    
    _PRINTF(L" >>> SEARCHSTRING: %ls->%ls\n",insstr,inreg);
    if (cfgm->search_ignorecase) {
        flags |= REG_ICASE;
    }
    if (cfgm->search_search) { // REGEX & WILDCARD
        flags |= REG_EXTENDED;
    } else { // LITERAL
        flags |= REG_LITERAL;        
    }
    
    ret = tre_regwcomp(&preg,inreg,flags);
    
    return ret;
}

int search_Destroy(void) {
    regfree(&preg);    
    return 0;
}

wchar_t* search_ErrDecode(int errcode) {
	char tmp[1024];
	errmsg[0] = L'\0';
    if (errcode) {          
        tre_regerror(errcode,&preg,tmp,1024);
		MultiByteToWideChar(CP_ACP,0,tmp,-1,errmsg,1024);
	}	
    return errmsg;
}

t_ergebnis *search_Search(void) {
    regmatch_t pmatch[1];
    regaparams_t aparam;
    regamatch_t pamatch[1];
    t_cfgmain *cfgm;
    t_ergebnis *ret;
    t_ergebnis *neu;
    int errcode = REG_OK;
    int i;
    int cnt=0;
    
    // main cfg
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    // search
    ret = NULL;
    i = 0;
    while (errcode == REG_OK) {
        errcode = tre_regwnexec(&preg,(cfgm->data)+i,(cfgm->datalen)-i,1,pmatch,0);
        if (errcode == REG_OK) {
            neu = erg_Create(i+pmatch[0].rm_so,i+pmatch[0].rm_eo);
            if (ret == NULL) {
                ret = neu;
            } else {
                erg_Append(ret,neu);
            }
            i += pmatch[0].rm_eo;            
            _PRINTF(L"%ls\n",(cfgm->data)+neu->abs_start);
            cnt++;
            if (cnt>=cfgm->resultsmax)
                break;
        }
    }
    //#warning approximate search is super slow and causes program crash :-(
	// ME 240926: stopped development here maybe at a later time to be implemented...
    /*_PRINTF(L">>> (%i) Fundstellen <<<\n",cnt);
    if (cfgm->search_approx && cnt<cfgm->resultsmin) {
        _PRINTF(L">>> Starte unscharfe Suche <<<\n");
        aparam.cost_ins = 10;
        aparam.cost_del = 10;
        aparam.cost_subst = 10;
        aparam.max_cost = 15;
        aparam.max_ins = 1;
        aparam.max_del = 1;
        aparam.max_subst = 1;
        aparam.max_err = 1;
        pamatch[0].pmatch = pmatch;
        
        errcode = REG_OK;
        i=0;
        while (errcode == REG_OK) {
            errcode = tre_regawnexec(&preg,(cfgm->data)+i,(cfgm->datalen)-i,pamatch,aparam,0);
            if (errcode == REG_OK) {
                _PRINTF(L".");
                neu = erg_Create(i+pamatch[0].pmatch->rm_so,i+pamatch[0].pmatch->rm_eo);
                if (ret == NULL) {
                    ret = neu;
                } else {
                    erg_Append(ret,neu);
                }
                i += pamatch[0].pmatch->rm_eo;            
                _PRINTF(L"%ls\n",(cfgm->data)+neu->abs_start);
                cnt++;
                if (cnt>=cfgm->resultsmax)
                    break;
            }
        }
    }
	*/
    
    _PRINTF(L">>> (%i) results found <<<\n",/*search_Count(ret)*/cnt);
    
    return ret;
}

int search_Count(t_ergebnis *erg) {
    int ret = 0;
    while (erg != NULL) {
        erg = erg->next;
        ret++;
    }
    return ret;
}

void search_Sort(t_ergebnis *ergebnisse) {
    // search result sorting with simple bubble sort
    int sorted = 0;
    int i;
    t_ergebnis *cur;
    
    while (!sorted) {
        sorted = 1;
        cur = ergebnisse;
        while (cur != NULL) {
            if (cur->next == NULL) {
                break;
            }
            if (cur->rate > (*(cur->next)).rate) {
                sorted = 0;
                // abs_start
                i = cur->abs_start;
                cur->abs_start = (*(cur->next)).abs_start;
                (*(cur->next)).abs_start = i;
                // offset_start
                i = cur->offset_start;
                cur->offset_start = (*(cur->next)).offset_start;
                (*(cur->next)).offset_start = i;
                // offset_stop
                i = cur->offset_stop;
                cur->offset_stop = (*(cur->next)).offset_stop;
                (*(cur->next)).offset_stop = i;
                // rate
                i = cur->rate;
                cur->rate = (*(cur->next)).rate;
                (*(cur->next)).rate = i;
            }
            cur = cur->next;
        }
    }
}

void erg_Delete(t_ergebnis *erg) {
    // delete result chain
    t_ergebnis *cur;
    
    cur = erg;
    while (cur != NULL) {
        erg = cur->next;
        free(cur);
        cur = erg;
    }
}

t_ergebnis* erg_Create(int abs_start,int abs_stop) {    
	// create result struct with data from libtre
    t_cfgmain *cfgm;
    t_ergebnis *ret;
    
    cfgm = (t_cfgmain*)cfgmain_Get();
    
    ret = (t_ergebnis*)malloc(sizeof(t_ergebnis));
    if (ret != NULL) {
        ret->abs_start = abs_start;
        ret->offset_start = abs_start;
        ret->offset_stop = abs_stop;
        while (ret->abs_start > 0) {
            if (cfgm->data[ret->abs_start-1] == '\0')
                break;
            ret->abs_start--;
        }
        ret->rate = abs_start - ret->abs_start;
        ret->next = NULL;
    }
    return ret;
}

void erg_Append(t_ergebnis *chain,t_ergebnis *erg) {
    if (chain == NULL) 
        return;
    while (chain->next != NULL) 
        chain = chain->next;
    chain->next = erg;
}
