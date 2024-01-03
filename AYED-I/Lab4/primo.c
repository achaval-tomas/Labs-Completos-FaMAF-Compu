#include <stdio.h>
#include <math.h>

int nesimo_primo(int N){
    int i, res;
    int cont = 0;
    int b = 2;
    while (cont < N){
    i = 2;
    res = 1;
    
    while ( (i <= (int)(sqrt(b)+1)) && res) {
        res = res && ((b % i) != 0);
        i = i+1;
    }                                       // a esta altura i será igual a sqrt(b)+2 sii b es primo.
    if (i == (int)(sqrt(b) + 2)){cont++;}   // si b es primo, aumento en 1 el contador.
    b = b+1;                                // analizo el numero siguiente a b.
}

return b-1; // cuando el contador llegue a N, significa que ultimo
}           // valor analizado (b-1) es el n-ésimo primo, y lo devuelvo.

int main (void){
    int n;
    printf("Introduzca el valor de n\n");
    scanf("%d", &n);
    
    while (n < 0){
        printf("Error. Introduzca un nuevo valor, positivo, para n\n");
        scanf("%d", &n);
    }
    
    printf("\nEl primo numero %d es %d\n", n, nesimo_primo(n));
 return 0;   
}
