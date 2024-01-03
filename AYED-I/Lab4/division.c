#include <stdio.h>
#include <assert.h>

// defino una estructura.
struct div_t {
int cociente;
int resto;
};

// defino una funcion que utiliza el algoritmo de la division.
struct div_t division(int x, int y){
    int i = 0;

    while (y<=x){
        x = x - y;
        i++;
    }

// armo una estructura con los valores obtenidos de cociente y resto.
    struct div_t p;
    p.cociente = i;
    p.resto = x;

return p;
}

// funcion main que solicita valores para realizar una division entera.
int main(void){
    int x, y;

    printf("Introduzca sus valores de la forma \"dividendo, divisor\"\n");
    scanf("%d, %d", &x, &y);
    assert(x>=0 && y > 0); // aseguro que el usuario haya ingresado valores correctos.

    printf("Cociente = %d, Resto = %d\n", division(x, y).cociente, division(x, y).resto); // imprimo el resultado final

return 0;
}

