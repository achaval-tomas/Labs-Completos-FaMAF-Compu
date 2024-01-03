#include <stdio.h>

int minimo (int x, int y){
int minimo = 0;
if (x<=y){minimo=x;}
else{minimo=y;}
return minimo;
}

int main (void){
int a, b;
printf("Introduzca a, b\n");
scanf("%d, %d", &a, &b);
printf("El minimo es %d\n", minimo(a, b));
return 0;
}

