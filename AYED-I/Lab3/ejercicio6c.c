#include <stdio.h>
#include "entradas.c"

/*Redefiniciones de algunos ejercicios*/

int main (void){
    int x, i,res;
    i = 2;
    res = 1;
    
    /*Solicito el valor de x al usuario*/
    x = pedirEntero();
    
    while ( (i < x) && res) {
        res = res && ((x % i) != 0);
        i = i+1;
        printf("x = %d, i = %d, res = %d\n", x, i, res);
    }
    
    if (i == x) {
        printf("%d es un numero primo!\n", x);
    } else {
        printf("El divisor mas chico de %d es ", x);
        imprimeEntero(i-1);
        }
        
    /*El programa encuentra el divisor distinto de 1 mas chico de un numero*/
    return 0;
}
