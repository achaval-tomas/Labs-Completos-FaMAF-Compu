#include <stdio.h>

int valorabsoluto(int x){
int abs = 0;
if (x>=abs){abs=x;}
else {abs = -x;}
return abs;
}

int main(void){
int x;
printf("Introduzca el valor de x\n");
scanf("%d", &x);
printf("El valor absoluto de %d es %d\n", x, valorabsoluto(x));
return 0;
}


