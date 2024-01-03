#include <stdio.h>

/* pedir e imprimir arreglos */
void pedirArreglo(int a[], int n_max){
    printf("Introduzca los %d elementos separados por espacios: ", n_max);
    for (int i = 0; i < n_max; i++){
    scanf("%d", &a[i]);
    }
}

void imprimeArreglo(int a[], int n_max){
    printf("[");
    for (int i = 0; i < n_max; i++){
    printf("%d", a[i]);
    if (i<n_max-1){printf(", ");}
    }
    printf("]");
}

/* pedir e imprimir enteros */
int pedirEntero (void){
    int a;
    printf("Introduzca un numero entero:\n");
    scanf("%d", &a);
    return a;
}

void imprimeEntero (int x){
    printf("%d\n", x);
}

/* librerias */
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

/* funciones */
typedef int numeros;
typedef struct p {
    numeros maximo;
    char letras;
} estructura;

