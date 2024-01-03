#include <stdio.h>
/* Ejercicio 3 */
int main (void) {
    int x,y;
    char z;
    /*Primero permito que el usuario decida que programa utilizará*/
    printf("Queres correr el a, b o el c?\n");
    scanf("%c",&z);
    
    /*Cada programa está definido en su propia guarda*/
    if (z =='a'){
    printf("Introduzca el valor de x para el estado incial!\n");
    scanf("%d", &x );
    x = 5;
    printf("Tu nuevo estado es x = %d", x);
         } 
         
        else if (z == 'b') {
          printf("Introduzca el valor de x, y para el estado incial!\n");
          scanf("%d, %d", &x, &y );
          x = x+y;
          y = y+y;
          printf("Tu nuevo estado es x = %d, y = %d\n", x, y);
           } 
           
            else if (z == 'c') {
              printf("Introduzca el valor de x, y para el estado incial!\n");
              scanf("%d, %d", &x, &y );
              y = y+y;
              x = x+y;
              printf("Tu nuevo estado es x = %d, y = %d\n", x, y);
                }
                
                 else {
                    printf("Ese no es un programa! Intenta de nuevo.\n");
                 }
    return 0;
    }
    
/*
1.a ejecucion 1
Introduzca el valor de x para el estado incial!
9
Tu nuevo estado es x = 5

1.a ejecucion 2
Introduzca el valor de x para el estado incial!
12
Tu nuevo estado es x = 5

1.a ejecucion 3
Introduzca el valor de x para el estado incial!
100
Tu nuevo estado es x = 5


1.b ejecucion 1
Introduzca el valor de x, y para el estado incial!
2, 5
Tu nuevo estado es x = 7, y = 10

1.b ejecucion 2
Introduzca el valor de x, y para el estado incial!
5, 7
Tu nuevo estado es x = 12, y = 14

1.b ejecucion 3
Introduzca el valor de x, y para el estado incial!
12, 4
Tu nuevo estado es x = 16, y = 8

1.c ejecucion 1
Introduzca el valor de x, y para el estado incial!
2, 5
Tu nuevo estado es x = 12, y = 10

1.c ejecucion 2
Introduzca el valor de x, y para el estado incial!
5, 7
Tu nuevo estado es x = 19, y = 14

1.c ejecucion 3
Introduzca el valor de x, y para el estado incial!
12, 4
Tu nuevo estado es x = 20, y = 8

*/
   
    
