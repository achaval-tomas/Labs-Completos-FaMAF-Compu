#include <stdio.h>
int main (void) {
    int x,y,z,b,w;
     /* Solicita los valores de todas las variables para los programas de los ejs. 1 y 2 */
    printf("Introduzca el valor de x:\n");
    scanf("%d", &x);
    printf("Introduzca el valor de y:\n");
    scanf("%d", &y);
    printf("Introduzca el valor de z:\n");
    scanf("%d", &z);
    printf("Introduzca el valor de b:\n");
    scanf("%d", &b);
    printf("Introduzca el valor de w:\n");
    scanf("%d", &w);
    
    /*Imprime los resultados de las operaciones de los ejercicios 1*/
    printf("\nLos resultados de las ecuaciones del 1 son:\n");
    printf("x+y+1 = %d\n", x+y+1);
    printf("(z*z) + (y*45) - (15*x) = %d\n",  (z*z) + (y*45) - (15*x));
    printf("(y-2) == (x*3 + 1) % 5 = %d\n", (y-2) == (x*3 + 1) % 5);
    printf("y/2*x = %d\n", y/2*x);
    printf("y < (x*z) = %d\n\n", y < (x*z));
    
    /*Imprime los resultados del 2*/
    printf("Y para las del 2 los valores son:\n");
    printf("(x % 4) == 0 -> %d\n", (x % 4) == 0);
    printf("(x+y) == 0 && (y-x) == (-1)*z -> %d\n", (x+y) == 0 && (y-x) == ((-1)*z));
    printf("(!b) && w -> %d\n", (!b) && w);
    printf("Recorda que 0 es False y 1 es True!");
    
    return 0;
    }

/* Consigna 1:
    
    para x=7, y=3, z=5,
    x+y+1 = 11
    (z*z) + (y*45) - (15*x) = 55
    (y-2) == (x*3 + 1) % 5 = 0
    y/2*x = 7
    y < (x*z) = 1
    
    para x=1, y=10, z=8,
    x+y+1 = 12
    (z*z) + (y*45) - (15*x) = 499
    (y-2) == (x*3 + 1) % 5 = 0
    y/2*x = 5
    y < (x*z) = 0
*/

/* Consigna 2:
    x=4, y=-4, z=8,b=0, w=0
*/
