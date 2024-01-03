#include <stdio.h>
int pedirEntero (void){
    int a;
    printf("Introduzca un numero entero:\n");
    scanf("%d", &a);
    return a;
}

void imprimeEntero (int x){
    printf("%d\n", x);
}
/*
int main (void){ 
    imprimeEntero(pedirEntero());
    return 0;
}
*/
