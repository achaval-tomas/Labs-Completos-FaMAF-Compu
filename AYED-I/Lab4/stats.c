#include <stdio.h>
#include <limits.h>

// funcion modificada traida de ejercicio anterior:
float * pedirArreglo(float a[], int n_max){
    for (int i = 0; i < n_max; i++){
    printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%f", &a[i]);
    }
    return a;
}

// defino una estructura de datos.
struct datos_t {
float maximo;
float minimo;
float promedio;
};

struct datos_t stats(float a[], int tam){
    // asigno y defino elementos necesarios.
    struct datos_t x;
    float sum = 0;
    x.maximo = INT_MIN;
    x.minimo = INT_MAX;
    
    // un unico ciclo que determina y almacena el maximo, minimo y la suma del arreglo.
    for (int i = 0; i < tam; i++){
        if (a[i] >= x.maximo){
            x.maximo = a[i]; 
        }
        
        if (a[i] <= x.minimo){
            x.minimo = a[i];
        }
        
        sum = a[i] + sum;
    }
    // calculo el promedio con la suma/tamaño del arreglo.
    x.promedio = sum/tam;

    return x;
}

/* funcion main que solicita el tamaño de un arreglo, sus elementos, y devuelve
sus datos minimo, maximo y promedio */
int main (void){
    int n;
    printf("Cual es el tamaño de tu arreglo?\n");
    scanf("%d", &n);
    
    float a[n];
    pedirArreglo(a, n);
    
    struct datos_t k = stats(a, n); // simplifico stats(a, n) para poder acceder mas sencillo con k.dato
    printf("\nEl maximo es %f, el minimo es %f, y el promedio es %f\n", k.maximo, k.minimo, k.promedio);
 
 
 return 0;   
}
