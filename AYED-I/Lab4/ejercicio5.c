#include <stdio.h>

// funcion que pide los elementos de un arreglo, sabiendo su tamaño, para crearlo.
int * pedirArreglo(int a[], int n_max){
    printf("Introduzca los %d elementos separados por espacios: ", n_max);
    for (int i = 0; i < n_max; i++){
    // printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%d", &a[i]);
    }
    return a;
}

// funcion que imprime todos los valores de un arreglo con la estructura "tipica" [ ... ].
void imprimeArreglo(int a[], int n_max){
    printf("[");
    for (int i = 0; i < n_max; i++){
    printf("%d", a[i]);
    if (i<n_max-1){printf(", ");}
    }
    printf("]");
    }
    
// funcion main que solicita el tamaño de un arreglo y combina las anteriores.
int main (void){
    int a[] = {};
    int n_max;
    printf("Introduzca el tamaño de su arreglo: ");
    scanf("%d", &n_max);
    imprimeArreglo(pedirArreglo(a, n_max), n_max);
    return 0;
    }

