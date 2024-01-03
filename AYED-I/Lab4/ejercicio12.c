#include <stdio.h>
#include <stdbool.h>

typedef char clave_t;
typedef int valor_t;

struct asoc {
 clave_t clave;
 valor_t valor;
};

// funcion modificada traida de ejercicio anterior:
void pedirArreglo(struct asoc a[], int tam){
    
    for (int i = 0; i < tam; i++){
    printf("Introduzca los datos \"valor, clave\" en la posicion %d del arreglo:\n", i);
    scanf("%d, %c", &a[i].valor, &a[i].clave);
    }
}


bool asoc_existe(struct asoc a[], int tam, clave_t c){
    bool b = false;
    
    for (int i = 0; i < tam; i++){
        b = (a[i].clave == c) || b;
    }
    
    return b;
}

int main (void){
    clave_t key;
    struct asoc a[6];
    
    pedirArreglo(a, 6);
    printf("Que clave desea verificar?\n");
    scanf(" %c", &key);

    if (asoc_existe(a, 6, key)){
        printf("La clave %c si existe en el arreglo.\n", key);
    } else {
        printf("La clave %c no existe en el arreglo.\n", key); 
    }
    
 return 0;   
}
