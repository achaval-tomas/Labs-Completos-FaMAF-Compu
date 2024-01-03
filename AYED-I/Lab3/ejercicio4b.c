#include <stdio.h>
int main (void){
    int x,y,z;
    int m = 0;
    /*Obtengo los valores de las variables*/
    printf("Introduzca valores de x, y, z:\n");
    scanf("%d, %d, %d", &x, &y, &z); 
    
    if (x < y) {
        m = x;
    } else {
        m = y;
    } 
     /*Imprimo el estado intermedio*/
    printf("\nEl estado medio es x = %d, y =  %d, z = %d, m = %d\n\n", x, y, z, m);
    
    if (m < z) {} else { m = z; }
    
     /*Imprimo el estado final*/
    printf("El estado final es x = %d, y =  %d, z = %d, m = %d\n", x, y, z, m);
    return 0;
}
/*
Introduzca valores de x, y, z:
5, 4, 8

El estado medio es x = 5, y =  4, z = 8, m = 4

El estado final es x = 5, y =  4, z = 8, m = 4
*/

/* Este programa determina el minimo entre los valores x, y, z y lo almacena en m */
