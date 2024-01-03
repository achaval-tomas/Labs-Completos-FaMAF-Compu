#include <stdio.h>
#include <limits.h>

// funcion traida del ejercicio anterior:
// funcion que pide los elementos de un arreglo, sabiendo su tamaño, para crearlo.
int * pedirArreglo(int a[], int n_max){
    for (int i = 0; i < n_max; i++){
    printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%d", &a[i]);
    }
    return a;
}

// funcion que calcula y devuelve el elemento par mas chico de un arreglo.
int minimo_pares(int a[], int tam){
    int min = INT_MAX;
    for (int i = 0; i < tam; i++){
        if (a[i] < min && a[i]%2 == 0){
            min = a[i];
        }
    }
    return min;
}

// funcion que calcula y devuelve el elemento impar mas chico de un arreglo.
int minimo_impares(int a[], int tam){
    int min = INT_MAX;
    for (int i = 0; i < tam; i++){
        if (a[i] < min && a[i]%2 != 0){
            min = a[i];
        }
    }
    return min;
}

// funcion main que solicita un arreglo e imprime en pantalla el elemento mas chico.
int main (void){
    int a[10];
    pedirArreglo(a, 10);
    
    if (minimo_pares(a, 10) < minimo_impares(a, 10)){
        printf("El minimo es el numero par %d", minimo_pares(a, 10));
    } else {
        printf("El minimo es el numero impar %d", minimo_impares(a, 10));
    }
    
    return 0;
}
