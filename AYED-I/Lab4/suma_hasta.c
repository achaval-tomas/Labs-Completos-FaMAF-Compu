#include <stdio.h>

int suma_hasta(int N){
    int i = 0;
    int sum = 0;
    while (i<N){
        sum = sum + i;
           i++;
   }
return sum;
}

int main (void){
    int N;
    printf("Introduzca el valor de N\n");
    scanf("%d", &N);
    
    if (N>0){
        printf("La suma de los naturales hasta N es %d\n", suma_hasta(N));
    } else { 
        printf("Error. Introduzca un numero positivo.\n");
    } 
return 0;
}
