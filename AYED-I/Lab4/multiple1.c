#include <stdio.h>

int main (void){
int equis, i;
printf("Introduzca los valores de x, y\n");
scanf("%d, %d", &equis, &i);
int x = equis + 1;
int y = equis + i;
printf("El estado final es x = %d, y = %d\n", x, y);
return 0;
}
