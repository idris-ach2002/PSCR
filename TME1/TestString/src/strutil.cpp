// strutil.cpp
#include "strutil.h"

namespace pr {

    size_t length(const char* s) {
        const char * curseur = s;
        while(*curseur != '\0') curseur ++;
        return (curseur - s);
    }

    char* newcopy(const char* s) {
        size_t len = length(s);
        char * res = new char[len + 1];

        //Déclaration des curseurs
        const char * crs1 = s;
        char * crs2 = res;

        while(*crs1 != '\0') {
            *crs2 = *crs1;
            crs1++;
            crs2++;
        }
        //Copier le '\0'
        *crs2 = *crs1;
        
        return res;
    }

    /*
    n C et C++, on a trois types de caractères différents :

    signed char

    unsigned char

    char (tout court)

    Problème : le standard dit que char doit être un octet, mais il ne dit pas si c’est signé ou non signé.
        
    Le caractère é en encodage ISO-8859-1 (Latin-1) a le code 233 en décimal.

    Cas 1 : char non signé

    Valeurs possibles : 0 … 255.

    Donc char c = 233; → c == 233.

    Cas 2 : char signé

    Valeurs possibles : -128 … 127.

    Le binaire 11101001 est interprété comme un nombre négatif en complément à deux.

    Calcul : 233 - 256 = -23.

    Donc char c = 233; → c == -23.


    ************************

    compare("é", "z"); 

    "é" → premier caractère vaut 233 si char non signé, ou -23 si char signé.

    "z" → 'z' = 122.

    👉 Si char est signé :

        -23 - 122 = -145 → indique "é" < "z".

    👉 Si char est non signé :

        233 - 122 = +111 → indique "é" > "z".
    */

    int compare(const char* a, const char* b) {
        while(*a && (*a == *b)) {
            a++;
            b++;
        }
        return (unsigned char)(*a) - (unsigned char)(*b);    
    }

    char * newcat (const char *a, const char *b) {
        char * res = new char[length(a) + length(b) + 1];
        
        char * ptr = res;
        //On recopie a dans res
        while(*a) {
            *ptr = *a;
            a++;
            ptr++;
        }

        //On recopie b dans res
        while(*b) {
            *ptr = *b;
            b++;
            ptr++;
        }

        //On recopie '\0' dans res
        *ptr = '\0';

        return res;
    }

}
