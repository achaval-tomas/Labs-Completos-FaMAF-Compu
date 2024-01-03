#include <stdio.h>

int main (void){
int equis, i, ceta; //creo variables temporales
printf("Introduzca los valores de x, y, z\n");
scanf("%d, %d, %d", &equis, &i, &ceta);
int x = i;
int y = i + equis + ceta;
int z = i + equis;
printf("El estado final es x = %d, y = %d, z =%d.\n", x, y, z); 
return 0;
}
