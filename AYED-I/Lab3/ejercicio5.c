#include <stdio.h>
int main (void){
    int i;
    /*Obetengo el valor de la variable*/
    printf("Introduzca el valor para i.\n");
    scanf("%d", &i);   
       
    /*5.a.h*/                          
    while (i != 0) {
        i = i - 1;
        /*Imprimo todos los estados por los que pasa*/
        printf("i = %d\n", i);
    }   
    
    /*5.a.i*/  
 /*   while (i != 0) {
        i = 0;                  
        printf("El nuevo valor de i es %d.", i); 
    }                                                */

    return 0;
    }
