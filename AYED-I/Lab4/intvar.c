#include <stdio.h>

int main (void){
int tempx, tempy; //variables temporales para evitar problemas de orden
printf("Introduzca valores para x, y\n");
scanf("%d, %d", &tempx, &tempy);
int x = tempy;
int y = tempx;
printf("El estado final es x = %d, y = %d\n", x, y);

return 0;
}
