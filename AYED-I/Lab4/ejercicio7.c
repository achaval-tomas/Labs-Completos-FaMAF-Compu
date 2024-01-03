#include <stdio.h>
#include <stdbool.h>

// funcion traida de ejercicio anterior:
// funcion que pide los elementos de un arreglo, sabiendo su tamaño, para crearlo.
void pedirArreglo(int a[], int n_max){
    for (int i = 0; i < n_max; i++){
    printf("Introduzca el valor en la posicion %d del arreglo:\n", i);
    scanf("%d", &a[i]);
    }
}

// funcion que decide si existe al menos un numero positivo en el arreglo.
bool existe_positivo(int a[], int tam){
    bool x = false;
    for (int i = 0; i < tam; i++){
    x = (a[i] > 0 ) || x ;
    if (x){i = tam;}  //corto el ciclo si encuentro un valor positivo.
    }
    return x;
}

// funcion que decide si todos los elementos del arreglo son positivos.
bool todos_positivos(int a[], int tam){
    bool x = true;
    for (int i = 0; i < tam; i++){
    x = (a[i] > 0 ) && x ;
    if (!x){i = tam;}  //corto el ciclo si encuentro un valor negativo.
    }
    return x;
}

// funcion main que utiliza las funciones anteriores para devolver una respuesta completa.
int main (void){
    int c;
    int a[10];
    pedirArreglo(a, 10);
    
    printf("Desea ver si existen positivos (1) o si todos son positivos (2)?\n");
    scanf("%d", &c);
    
    bool t;
    if (c == 1){
       t = existe_positivo(a, 10);
    } else if (c == 2){
       t = todos_positivos(a, 10);
    } 
    while (c != 1 && c!= 2) {
        printf("Esa funcion no existe, intentelo de nuevo.\n");
        scanf("%d", &c);
    }
    
    if (c == 1 && t){
      printf("Existe al menos un numero positivo en el arreglo!\n");
    } else if (c == 1 && !t){
      printf("No existe ningun numero positivo en el arreglo!\n");
    } else if (c == 2 && t){
      printf("Todos los numeros del arreglo son positivos!\n");
    } else if (c == 2 && !t){
      printf("Hay al menos un numero negativo en el arreglo!\n");
    }
    
 return 0;   
}
