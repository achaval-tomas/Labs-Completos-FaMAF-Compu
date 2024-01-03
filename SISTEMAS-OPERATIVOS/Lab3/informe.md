# Welcome to our magic

## Primera Parte

<b> 1. ¿Qué política de planificación utiliza xv6-riscv para elegir el próximo proceso a ejecutarse? </b>
Round Robin.

<b> 2. ¿Cuánto dura un quantum en xv6-riscv? </b>
1.000.000 ciclos.

<b> 3. ¿Cuánto dura un cambio de contexto en xv6-riscv? </b>
Dura 29 instrucciones


<b> 4. ¿El cambio de contexto consume tiempo de un quantum? </b>
Sí

<b> 5. ¿Hay alguna forma de que a un proceso se le asigne menos tiempo? Pista: Se puede empezar a buscar desde la system call uptime. </b>
Sí, podríamos guardar en el struct proc el tick en el que comenzó a ejecutar el proceso, y cuando haya una interrupción por timer
revisar cuántos ticks pasaron desde que comenzó a ejecutar. Así podríamos implementar una cola de prioridad en mlfq, donde a los
diferentes procesos se les asigna un tiempo diferente, pero siempre múltiplo del timer. No se le puede asignar menos de lo que tarda
el timer interrupt.

<b> 6. ¿Cúales son los estados en los que un proceso pueden permanecer en xv6-riscv y qué los hace cambiar de estado? </b>


Estados posibles: UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE.

Cuando ocurre una interrupción por timer se llama a yield(), y esta pasa al proceso de RUNNING a RUNNABLE.
Cuando el scheduler selecciona un proceso para ejecutar, este lo pasa de RUNNABLE a RUNNING.
La función wakeup() es la encargada de pasar los procesos que están esperando en un channel de SLEEPING a RUNNABLE.
Cuando ocurre una interrupción por I/O, se llama sleep(), que pasa un proceso de RUNNING a SLEEP.

Cuando un proceso ejecuta exit(), se pone en estado ZOMBIE, hasta que su padre llame a wait(), entonces el proceso se desaloca
y pasa a UNUSED.

La función allocproc() es la encargada de buscar un proceso UNUSED y pasarlo a USED.
El fork(), agarra un proceso nuevo y lo pasa de UNUSED a RUNNABLE.



Estado    - funcion que lo cambia de ese estado.
UNUSED    - allocproc(), fork()
USED      - fork() 
SLEEPING  - kill(), wakeup()
RUNNABLE  - scheduler()
RUNNING   - sleep(), exit(), yield()
ZOMBIE    - wait()

## Segunda Parte
[ANALISIS DE DATOS OBTENIDOS](https://docs.google.com/spreadsheets/d/1DC3cQIlHqiRpTnVMe6cQt5XA2VlFrMtwV3-KktBPdRQ/edit?usp=sharing)

## Cuarta parte
Sí se puede producir starvation, cuando un proceso va a la prioridad más baja y hay muchos procesos en prioridades más altas.
Una forma de mitigar esto es con priority boost.

## Conclusión
Es claro que la política de scheduling no influye cuando el benchmark que estamos corriendo tiene un solo proceso.

En el caso ```cpubench &; cpubench``` tampoco importa mucho, ya que en MLFQ ambos tendrán la menor prioridad, entonces irá alternando entre uno y otro al estilo RR.

En el caso ```cpubench &; iobench``` la diferencia no es mucha porque el iobench se la pasa mucho tiempo bloqueado por IO,
entonces corre el cpubench durante todo un quanto antes de volver al iobench. Sin embargo, se puede notar en el benchmark hecho
usando MLFQ y quanto normal, cómo la cantidad de veces que fue seleccionado el iobench fue mayor.

El caso ```cpubench &; cpubench &; iobench``` es en el que más se nota la diferencia de schedulers. Con RR cada proceso es elegido
aproximadamente la misma cantidad de veces. Sin embargo, con MLFQ, el iobench siempre tiene prioridad por sobre los dos cpubench, 
entonces el iobench es elegido la mitad de las veces, mientras que la otra mitad va alternando entre los dos cpubench.

| iobench  | cpubench0 | cpubench1  |  T  |
|----------|-----------|------------|-----|
| RUN      | READY     | READY      |  1  |
| BLOCKED  | RUN       | READY      |  2  |
| RUN      | READY     | READY      |  3  |
| BLOCKED  | READY     | RUN        |  4  |
| RUN      | READY     | READY      |  5  |
| BLOCKED  | RUN       | READY      |  6  |
| RUN      | READY     | READY      |  7  |
| BLOCKED  | READY     | RUN        |  8  |
| RUN      | READY     | READY      |  9  |

Representación de scheduling de ```cpubench &; cpubench &; iobench``` con MLFQ.