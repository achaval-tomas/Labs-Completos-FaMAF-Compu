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
    for (unsigned int n = 0; n < longitud; n++){
        sum = sum + arr[n].peso;
    }
    
    float promedio = (float)sum/longitud;
    return promedio;
}

persona_t persona_de_mayor_edad(persona_t arr[], unsigned int longitud)
{
    persona_t eldest = arr[0];
   
        for (unsigned int n = 0; n<longitud; n++){
            if (eldest.edad <= arr[n].edad){
                eldest = arr[n];
            }
        }

    return eldest;
}

persona_t persona_de_menor_altura(persona_t arr[], unsigned int longitud)
{
    persona_t shortest = arr[0];
    
        for (unsigned int n = 0; n<longitud; n++){
            if (shortest.altura >= arr[n].altura){
                shortest = arr[n];
            }
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
