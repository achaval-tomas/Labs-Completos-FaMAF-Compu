#include <stdio.h>
int main (void){
    int x,i,res;
    i = 2;
    res = 1;
    /*Solicito el valor de x al usuario*/
    printf("Introduzca el valor de x:\n");
    scanf("%d", &x);
    
    while ( (i < x) && res) {
        res = res && ((x % i) != 0);
        printf("x = %d, i = %d, res = %d\n", x, i, res);
        i = i+1;
    }
    /*El programa encuentra el divisor distinto de 1 mas chico de un numero*/
    return 0;
}
