#include <stdio.h>
#include <assert.h>

// funcion traida del ejercicio anterior:
// funcion que pide los elementos de un arreglo, sabiendo su tamaño, para crearlo.
int * pedirArreglo(int a[], int n_max){
    for (int i = 0; i < n_max; i++){
    printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%d", &a[i]);
    }
    return a;
}

// funcion que imprime todos los valores de un arreglo con la estructura "tipica" [ ... ].
void imprimeArreglo(int a[], int n_max){
    printf("[");
    for (int i = 0; i < n_max; i++){
    printf("%d ", a[i]);
    }
    printf("]");
    }

// funcion que intercambia dos elementos de un arreglo entre si, y devuelve el arreglo modiicado.
int * intercambiar(int a[], int i, int j){
    int x = a[i];
    int y = a[j];
    a[i] = y;
    a[j] = x;
    return a;
}

int main (void){
  int i, j, tam;
  
  printf("Introduzca las posiciones \"i, j\" de los elementos del arreglo que quiere intercambiar\n");
  scanf("%d, %d", &i, &j);
  
  printf("Introduzca el tamaño del arreglo\n");
  scanf("%d", &tam);
  
  int a[tam];
  pedirArreglo(a, tam);
  
  assert( 0 <= i && i < tam && 0 <= j && j < tam);
  
  imprimeArreglo(intercambiar(a, i, j), tam);
  
 return 0;   
}
