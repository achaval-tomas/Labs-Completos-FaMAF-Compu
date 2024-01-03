#include <stdio.h>

// funcion traida del ejercicio anterior:
// funcion que pide los elementos de un arreglo, sabiendo su tamaño, para crearlo.
int * pedirArreglo(int a[], int n_max){
    for (int i = 0; i < n_max; i++){
    printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%d", &a[i]);
    }
    return a;
}

// calcula y devuelve la suma de todos los elementos de un arreglo.
int sumatoria(int a[], int tam){
    int sum = 0;
    for (int i = 0; i < tam; i++){
     sum = sum + a[i];
    }
    return sum;   
}

// solicita los valores de un arreglo de tamaño fijo al usuario, calcula su suma y la imprime.
int main (void){
    int a[8];
    pedirArreglo(a, 8);
    printf("La suma de los elementos del arreglo vale %d\n", sumatoria(a, 8));
}
