#include <stdio.h>
#include "entradas.c"
#include "entradas_bool.c"
/* redefino 4b */
int main (void){
    int m ;
    printf("Introduzca 3 enteros para encontrar el minimo:\n");
    int x = pedirEntero();
    int y = pedirEntero();
    int z = pedirEntero();
    
     if (x < y) { m = x; } else {m = y;} 
     /*Imprimo el estado intermedio*/
     printf("El minimo entre %d y %d es:", x, y);
     imprimeEntero(m);
     
     if (m < z) {} else { m = z; }
    /*Imprimo el estado final*/
    printf("Entonces minimo entre %d, %d y %d es:", x, y, z);
    imprimeEntero(m);
    
    return 0;
}
