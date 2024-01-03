# FAT FUSE Censurado
FaMAFyC - Sistemas Operativos


## Estructura del repositorio

Fuse, la librería que nos permite crear filesystems desde el espacio de usuario,
es lo suficientemente flexible como para permitir cualquier tipo de
funcionalidad, mientras que se implemente las operaciones requeridas en
la estructura `fuse_operations`. Para entender qué hace el sistema de archivo,
deben comenzar por enteder esta estructura.

Cuando se monta el sistema de archivo, se invocan a las funciones del archivo
`fat_fuse.c`, que creará un nuevo `fat_volume` a partir de la información de la
imagen. Durante este proceso, al leer la tabla FAT, el sistema construye el
árbol de directorios en un TAD `fat_tree`. Luego, se llama a la función
`fuse_main`, que efectivamente monta el nuevo sistema de archivos.

Una vez que el sistema está corriendo, cada vez que se invoca una llamada a
sistema que involucra el filesytem, fuse llama a las funciones implementadas en
`fat_fuse_ops.c`. En estas
funciones sólo se implementa la lógica de alto nivel del filesystem que asegura
la consistencia de directorios y archivos. Las operaciones de lectura y
escritura de bajo nivel están implementadas en `fat_file.c`.

Toda esta información es guardada en estructuras en memoria mientras el
filesystem está corriendo, y se destruyen cuando el proceso termina o cuando se
desmonta el sistema de archivos. Por ello, al implementar las operaciones de
escritura, debemos **mantener la consistencia entre la información en memoria y
la información en la imagen**.

Otras limitaciones del sistema son:
 * Sólo tiene soporte para FAT32
 * Sólo acepta nombres de 8 caracteres más una extensión de 3 caracteres. (No
  tiene soporte para VFAT).
 * Sólo soporta un único hilo.

#### Cómo compilar y correr

Para poder correr la implementación, deben instalar libfuse-dev glib y check.

      $ sudo apt-get install libfuse-dev

Luego, pueden correr todo con:

      $ mkdir mnt
      $ make
      $ ./fat_fuse -f ./mnt

Para desmontar el sistema, correr:
      $ fusermount -u ./mnt

Si el programa falla inesperadamente, el sistema de archivos puede no
desmontarse correctamente. En ese caso, es necesario desmontarlo manualmente
usando los comandos nativos del sistema operativo.

#### Cómo crear y montar una imagen de prueba

[Original
source](http://fejlesztek.hu/create-a-fat-file-system-image-on-linux/)

Para crear una imagen vacía en formato FAT32, primero crean un archivo file.img
con 35MB de "zeros" adentro (para FAT32 el mínimo de tamaño de un archivo son
alrededor de 33 MB):

    $ dd if=/dev/zero of=file.img count=35 bs=1M

Luego continuamos formateando la imagen con el formato correcto:

    $ mkfs.vfat -F 32 -v ./file.img

Luego, para montar la imagen y poder agregar cosas necesitamos usar la función
mount del sistema y dar permisos de escritura para todos los usuario al punto
de montaje (lo cual requiere permisos de sudo). Primero, creamos el directorio
en el cual vamos a ver los archivos montados.

    $ mkdir mnt

El siguiente comando monta nuestra imagen file.img en el directorio mnt, y
concede todos los permisos.

    $ sudo mount -t vfat file.img mnt/ -o umask=000

Agregamos algo a la imagen:

    $ echo "Hello, this is a file" > mnt/test_file

Por último, tenemos que desmontar la imagen:

    $ sudo umount mnt/

Si volvemos a montar la imagen, podremos ver nuevamente el archivo test_file.

Para correr los tests que chequean la funcionalidad del lab van a tener que correr

    make testfs

### Módulos

**fat_fuse.c**
Contiene la función main, y se encarga de llamar a fuse y montar el volúmen.

**fat_volume.c**
Contiene las funciones que permiten montar el sistema de archivos. Esta
operación involucra abrir un directorio de montaje con los permisos
correspondientes y leer los componentes del sistema de archivos FAT.
Entre ellos:
 - Parámetros de la BIOS
 - Sector de boot
 - Múltiples copias de la FAT (File Allocation Table)
También inicializa la estructura de datos `fat_tree` que representa el árbol
de directorios con el directorio raíz.

Esta información se almacena en la estructura de datos `fat_volume`.

**fat_fuse_ops.c**
Contiene las operaciones que implementan el comportamiento de las distintas llamadas a sistema relacionadas al sistema de archivos.

**fat_fs_tree.c**
Define el TAD `fat_tree` que abstrae el árbol de directorios.
Internamente, `fat_tree` utiliza un `h_tree` cuyos datos son de tipo `fat_file`.
`fat_tree` hace de interfaz entre el `h_tree`, que es un árbol genérico, y
`fat_fuse_ops`, manejando las funciones particulares de `fat_file`.
Esta estructura es una combinación de
árbol de búsqueda binario y listas enlazadas. Esto permite realizar búsquedas
por nombre de archivo en tiempo logaritmico, y al mismo tiempo poder recorrer
fácilmente los archivos de un directorio, y el path completo de un archivo
(directorios "ancestros").

**fat_file.c**
Define el TAD `fat_tree` que abstrae la información y las funciones necesarias para manipular archivos. Tiene una copia de la entrada de directorio leída del cluster de datos de su directorio padre.

**fat_table.c**
Define el TAD `fat_table`, que abstrae la lógica de las cadenas de clusters y las operaciones de escritura/lectura de la tabla FAT.

El resto de los archivos contienen funciones y estructuras de datos auxiliares.

#### Debuggeando el código

Fuse puede correrse tanto en background como en foreground. Si está en
background, al llamarse a la función `fuse_main` en `fat_fuse.c`, se pasa el
control de ejecución a la librería y se redirige en `stderr` y el `stdout`. Al
llamar a `fusermount` el control retorna a nuestra aplicación (que ya está
corriendo como daemon) y se desmonta el sistema.

Para facilitar el debugging, si llamamos a `fuse_main` con la opción `-f`,
podemos correr fuse en foreground y ver los prints que realizamos desde el
código.

Además de ello, el sistema cuenta con un macro DEBUG que puede utilizarse para
imprimir mensajes opcionales. Para desactivar estos mensajes, tienen que
compilar con el flag `-DNDEBUG`.

Para un nivel todavía mayor de debugging, pueden correr el código con `-d` y
mostrará los mensajes provenientes de Fuse.


#### Estilo de código con clang-format

https://www.electronjs.org/docs/development/clang-format

Después de agregar todos los cambios, antes de hacer un commit:

    $ git-clang-format *.c *.h

Y finalmente re-agregan los archivos que hayan sido cambiados. Con `git diff` pueden ver los cambios realizados por clang.


## Errores y cosas que faltan

 * La hora de los directorios y archivos creados sale mal.
 * Al crear archivos, si el cluster de datos del directorio se llena de entradas
   se devuelve un error. Es necesario implementar la lógica para buscar un
   nuevo cluster de datos.
 * Hay un error al intentar crear archivos o directorios con más de 8  caracteres. El directorio/archivo es creado con el nombre truncado a 8  caracteres. Luego FUSE trata de buscar el nuevo directorio/archivo con el nombre original y finalmente devuelve que un error, ya que no existe.
 * No hay soporte para otros tipos de archivos que no sean archivos o directorios.
 * No controlamos que no existan ciclos de subdirectorios, ni que los paths no tengan más de 4096 caracteres (segmentation fault).

### Notas adicionales (del repositorio original de fat-fuse)

 * The file allocation table is mapped into memory with mmap(). Other data in  the filesystem, such as cluster data, are read with pread().
 * Whenever the child of a directory is needed, all children are read at the  same time and inserted into a balanced binary tree of that directory's  children, sorted by name (including extension, if present). The children of  a directory are freed at a later time if they are no longer in use and the  number of allocated files has exceeded a soft limit.
 * Whenever a file with no current open file descriptors is opened, a table is  allocated to map cluster indices of that file to actual clusters. When a  read at a given offset is requested, all previous entries in the table up  until the entry needed to complete the read are read from the FAT. When a  file is closed for the last time, its table of clusters is freed.
 * The filesystem code is not thread-safe, so concurrent filesystem operations  will be serialized.
 * The filesystem code does not try to detect cyclic directory structures (which  are possible in FAT). It will just recurse indefinitely.
