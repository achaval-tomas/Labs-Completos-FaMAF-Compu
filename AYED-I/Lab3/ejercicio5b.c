#include <stdio.h>
int main (void){
    int x,y,i;
    i = 0;
    /*Solicito los valores de x e y al usuario*/
    printf("Introduzca los valores x, y.\n");
    scanf("%d, %d", &x, &y);
    
    while (x>=y) {
        x = x-y;
        i = i+1;
        printf("x = %d, y = %d, i = %d\n", x, y, i);
    }
    
    return 0;
}

/*
Introduzca los valores x, y.
13, 3
Iteracion 1: x = 10, y = 3, i = 1
Iteracion 2: x = 7, y = 3, i = 2
Iteracion 3: x = 4, y = 3, i = 3
Iteracion 4: x = 1, y = 3, i = 4
*/
