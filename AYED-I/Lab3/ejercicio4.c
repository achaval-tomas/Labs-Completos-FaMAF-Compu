#include <stdio.h>
int main (void){
    int x,y;
    /*Doy opcion al usuario a elegir el estado inicial*/
    printf("Introduzca un valor de la forma \"x, y\"!\n");
    scanf("%d, %d", &x, &y);
    
    if (x >= y) {
        x = 0;
       } else if (x <= y) {
            x = 2;
           }
           
    printf("Tu estado final es %d, %d\n", x, y);    
    return 0;
}
