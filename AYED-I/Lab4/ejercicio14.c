#include <stdio.h>

typedef struct _persona {
char *nombre;
int edad;
float altura;
float peso;
} persona_t;

float peso_promedio(persona_t arr[], unsigned int longitud)
{
    int sum = 0;
    for (unsigned int i = 0; i < longitud; i++){
        sum = sum + arr[i].peso;
    }
    
    float promedio = (float)sum/longitud;
    return promedio;
}

persona_t persona_de_mayor_edad(persona_t arr[], unsigned int longitud)
{
    unsigned int i = 0;    // creo una variable para utilizar en un ciclo.
    int b;                 // defino b que utilizaré como valor booleano para comapraciones. 
    persona_t eldest;     // defino una persona "eldest" la cual será devuelta por la funcion.

    while (i<longitud){
        b = 1;  // reseteo mi valor de referencia booleano antes de comenzar cada comparacion.

// A la edad de cada persona, la comapro con las edades de todas las personas del arreglo.    
        for (unsigned int n = 0; n<longitud; n++){
            b = (arr[i].edad >= arr[n].edad) && b;       
        }

/* En este momento b sera true sii la persona en posicion i tiene edad mayor
a todas las otras personas del arreglo. En dicho caso, asigno que "eldest"
sea la persona en la posicion i del arreglo, y corto el ciclo. */
        if(b){eldest = arr[i]; i = longitud;}

// En otro caso, continuo analizando.
        i++;
    }

    return eldest;
}

persona_t persona_de_menor_altura(persona_t arr[], unsigned int longitud)
{
    unsigned int i = 0;    // creo una variable para utilizar en un ciclo.
    int b;                 // defino b que utilizaré como valor booleano para comapraciones.
    persona_t shortest;    // defino una persona "shortest" la cual será devuelta por la funcion.
    
    while (i<longitud){
        b = 1; // reseteo mi valor de referencia booleano antes de comenzar cada comparacion.

// A la altura de cada persona, la comapro con las alturas de todas las personas del arreglo.        
        for (unsigned int n = 0; n<longitud; n++){      
            b = (arr[i].altura <= arr[n].altura) && b;      
        }

/* En este momento b sera true sii la persona en posicion i tiene altura menor 
a todas las otras personas del arreglo. En dicho caso, asigno que "shortest"
sea la persona en la posicion i del arreglo, y corto el ciclo. */
        if(b){shortest = arr[i]; i = longitud;}
        
// En otro caso, continuo analizando.
        i++;
    }

    return shortest;
}

int main(void) {
    persona_t p1 = {"Paola", 34, 1.65, 65};
    persona_t p2 = {"Luis", 27, 1.63, 69};
    persona_t p3 = {"Julio", 72, 1.75, 80};
    persona_t p4 = {"Alejandro", 60, 1.78, 83};
    persona_t p5 = {"Marisa", 44, 1.53, 54};
    unsigned int longitud = 5;
    persona_t arr[] = {p1, p2, p3, p4, p5};  // lista de 5 personas para probar mi funcion.
    
    printf("El peso promedio es %.2f\n", peso_promedio(arr, longitud));
    
    persona_t p = persona_de_mayor_edad(arr, longitud);
    printf("El nombre de la persona con mayor edad es %s\n", p.nombre);
    
    p = persona_de_menor_altura(arr, longitud);
    printf("El nombre de la persona con menor altura es %s\n", p.nombre);
    
    return 0;
}
