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

// defino la estructura.
struct comp_t {
int menores;
int iguales;
int mayores;
};

/* defino una funcion que dado un arreglo, su tamaño y un elemento "elem" a comparar,
organiza un arreglo con los elementos iguales, menores y mayores a elem! */
struct comp_t cuantos(int a[], int tam, int elem){
    struct comp_t s;
    s.iguales = 0;
    s.menores = 0;
    s.mayores = 0;
    
    for (int i = 0; i < tam; i++){
        if (a[i] == elem){
        s.iguales = 1 + s.iguales;
    } else if (a[i] < elem){
        s.menores = 1 + s.menores;
    } else if (a[i] > elem){
        s.mayores = 1 + s.mayores;
    }
    }
    
    return s;
}

/* funcion main que solicita el tamaño de un arreglo, sus elementos, un elemento a comparar
 y luego imprime en pantalla los elementos iguales, menores y mayores al mismo. */
int main (void){
    int n, elem;
    
    printf("Cual es el tamaño de tu arreglo?\n");
    scanf("%d", &n);
    
    int a[n];
    pedirArreglo(a, n);
    
    printf("Que elemento quieres comparar?\n");
    scanf("%d", &elem);
    
    struct comp_t k = cuantos(a, n, elem);
    printf("En el arreglo hay %d elementos iguales a %d,\n%d elementos mayores, y %d elementos menores!\n", k.iguales, elem, k.mayores, k.menores);
    
 return 0;   
}
