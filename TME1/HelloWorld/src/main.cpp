#include <iostream>
#include <stddef.h>
int main() {
    int tab[10];
    for (int i=9; i >= 0 ; i--) {
        if (tab[i] - tab[i-1] != 1) {
            std::cout << "probleme !";
        }
    }
    return 0;
}