#include <stdio.h>
#include <stdbool.h>

bool pedirBooleano(void) {
    bool b;
    int i;
    printf("Introduzca un valor booleano:\n");
    scanf("%d", &i);
    b = i;
    return b;
}

void imprimeBooleano(bool x){
    if (x){
        printf("verdadero");
    } else {
        printf("falso");
    }
}

int main (void){
    imprimeBooleano(pedirBooleano());
    return 0;
}

