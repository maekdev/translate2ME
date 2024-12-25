// cfg_parser.c
// 
// 130730
// markus ekler
//
// kleiner scanner/parser für konfigurationsdateien
//

#include "config.h"


// CTYP Enumeration
enum { C_NL,C_KOMMENTAR,C_LEER,C_CHAR,C_ZAHL,C_GLEICH,C_KLA,C_KLZ,C_SLASH,C_HOCHK,C_KOMMA }; 
// LAMBDA Aktion Enumeration
enum { L_NICHTS,L_PUSHID,L_PUSHSTR1,L_PUSHSTR2,L_TERMID,L_TERMSTR1,L_TERMSTR2,L_IRESET,L_IADD,L_TYPEEXP,L_TYPEFUNK0,L_TYPEFUNK1,L_TYPEFUNK2 };

int ctyp(wchar_t ch) {
	switch (ch) {
		case L'\0':
		case L'\r':
		case L'\n': return C_NL;
		case L' ': return C_LEER;
		case L'#': return C_KOMMENTAR;
		case L'"':
		case L'\'': return C_HOCHK;
		case L'(': return C_KLA;
		case L')': return C_KLZ;
		case L'=': return C_GLEICH;
		case L'\\': return C_SLASH;
		case L',': return C_KOMMA;
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9': return C_ZAHL;
	}
	return C_CHAR;
}

int scant_reset(t_scanner *sc) {
	sc->type = T_FEHLER;
	sc->i = 0;
}

int cfg_scanner(t_scanner *sc,wchar_t *buf,int buflen) {
    int ret;
    wchar_t *id = sc->id;
    wchar_t *str1 = sc->str1;
    wchar_t *str2 = sc->str2;
    int zustand = 0;

	int delta[16][11] = {
    //  NL, #, _,ch,za, =, (, ), \, ", ,,
         0, 1, 0, 2,16,16,16,16,16,16,16,   // 0
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   // 1
        22,16, 3, 2, 2, 4, 8,16,16,16,16,   // 2
        22,16, 3,16,16, 4, 8,16,16,16,16,   // 3
        16,16, 4,16, 5,16,16,16,16, 6,16,   // 4
        17,16,17,16, 5,16,16,16,16,16,16,   // 5
        16, 6, 6, 6, 6, 6, 6, 6, 7,18, 6,   // 6
        16, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,   // 7
        16,16, 8,16,16,16,16,19,16, 9,16,   // 8
        16, 9, 9, 9, 9, 9, 9, 9,10,11, 9,   // 9
        16, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,   // 10
        16,16,11,16,16,16,16,20,16,16,12,   // 11
        16,16,12,16,16,16,16,16,16,13,16,   // 12
        16,13,13,13,13,13,13,13,14,15,13,   // 13
        16,13,13,13,13,13,13,13,13,13,13,   // 14
        16,16,15,16,16,16,16,21,16,16,16 }; // 15
    int lambda[16][11] = {
    //  NL, #, _,ch,za, =, (, ), \, ", ,,
         0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,   // 0
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 1
         4, 0, 4, 1, 1, 4, 4, 0, 0, 0, 0,   // 2
         4, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0,   // 3
         0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,   // 4
         9, 0, 9, 0, 8, 0, 0, 0, 0, 0, 0,   // 5
         0, 2, 2, 2, 2, 2, 2, 2, 0, 5, 2,   // 6
         0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   // 7
         0, 0, 0, 0, 0, 0, 0,10, 0, 0, 0,   // 8
         0, 2, 2, 2, 2, 2, 2, 2, 0, 5, 2,   // 9
         0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   // 10
         0, 0, 0, 0, 0, 0, 0,11, 0, 0, 0,   // 11
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 12
         0, 3, 3, 3, 3, 3, 3, 3, 0, 6, 3,   // 13
         0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   // 14
         0, 0, 0, 0, 0, 0, 0,12, 0, 0, 0 }; // 15
         
	if (buflen == 0) {
		return -1;
	}
	scant_reset(sc);
    ret = 0;
    zustand = 0;
    while (zustand < 16 && ret < buflen) {
        // Aktion ausfuehren
        switch (lambda[zustand][ctyp(*buf)]) {
            case L_PUSHID:
                *id = *buf;
                id++;
                break;
            case L_PUSHSTR1:
                *str1 = *buf;
                str1++;
                break;
            case L_PUSHSTR2:
                *str2 = *buf;
                str2++;
                break;
            case L_TERMID:
                *id = L'\0';
                break;
            case L_TERMSTR1:
                *str1 = L'\0';
                break;
            case L_TERMSTR2:
                *str2 = L'\0';
                break;
            case L_IRESET:
                sc->i = 0;
                break;
            case L_IADD:
                sc->i *= 10;
                sc->i += (*buf)-L'0';
                break;          
        }
        // naechster zustand
        zustand = delta[zustand][ctyp(*buf)];
        buf++;
        ret++;
    }    
    sc->type = zustand;
    /*switch (zustand) {
        case 16: sc->type = T_FEHLER; break;
        case 17: sc->type = T_ZUWEISUNGINT; break;
        case 18: sc->type = T_ZUWEISUNGSTR; break;
        case 19: sc->type = T_FUNK0; break;
        case 20: sc->type = T_FUNK1; break;
        case 21: sc->type = T_FUNK2; break;
        case 22: sc->type = T_DEFINITION; break;
    }*/
    
    // im fehlerfall position zur naechsten NL springen
    while (ret < buflen) {
        if (ctyp(*buf) == C_NL) {
            break;
        }
        buf++;
        ret++;
    }
    
	return ret;
}
