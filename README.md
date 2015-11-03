# Proyecto find
1. Realiza una busqueda desde cualquier path, relativa o absoluta **(20 pts)**
2. La busqueda que realiza es recursiva (busca tambien dentro de las sub-carpetas) **(20 pts)**
3. Busca y compara cuatro atributos de un archivo: **(40 pts)**
  1. Nombre   **(10 pts)**
  1. Tipo     **(10 pts)**
  1. Dueño    **(10 pts)**
  1. Permisos **(10 pts)**
4. Se le puede mandar un nuevo comando al cual redireccionara lo que encuentre mediante exec **(20 pts)**

## Estructuras de datos globales
```c
    //Directorio en el cual se realizara la busqueda
    char * path
    //Atributos que el usuario quiere buscar
    char * atributo[4]
        1. Nombre
        2. Tipo
        3. Dueño
        4. Permisos
    //Comando a ejecutar y sus parametros
    char * exe[10]
    //Indica si el usuario pasó un comando a ejecutar posteriormente
    int hayexec = 0
```


## Main
- Si no se pasaron argumentos, se termina el programa
- evalParams()
- Si hay exec, preparaHijo()
- Lanzar hilo de busqueda()
- Se espera a que todos los hilos acaben




## Funcion: Analizar parametros
*Esta funcion es parte del punto 3 y 4, con un total de* **40 pts** 

Recibe `argc` y `argv[]`

No regresa nada

    Recorrer los elementos de argv[] con un contador i
        Si el elemento es:
            "-n", argumento[0] es argv[++i]
            "-t", argumento[1] es argv[++i]
            ...
            "-exec"
                hayexec = 1
                i++
                j=0
                mientras i<argc y j<10
                    exe[j++] = argv[i++]
            Ninguno de los anteriores, si path == NULL
                    path = argv[i]
                De no ser asi
                    Error de sintaxis
                    Terminar el programa

## Funcion: Buscar

*Esta funcion es parte del punto 1, 2, 3 y 4 con un total de* **100 puntos** 

Función que busca un archivo de acuerdo a una lista de atributos proporcionada por el usuario.

Recibe un directorio en el cual buscar

Utiliza la variable global `atributo[]`

No regresa nada

    encontrado = 1
    Abre la estructura del directorio proporcionado
    Por cada archivo en la estructura:
        Si es un directorio:
            Se lanza un nuevo hilo con la funcion Buscar
            Se pasa el nombre del directorio como parametro a Buscar
        Si es un archivo
            Por cada atributo no vacio en atributo[]
                Se recibe el atributo del struct archivo
                Si archivo.atributo != atributo[correspondiente]
                    encontrado = 0
        Si encontrado==1
            Se obtiene la ruta absoluta del path
            Se escribe a pantalla la ruta obtenida
            Si hayexec
                Se añade la ruta a exe[]
                Se executa el comando exe[0] con los parametros exe[]

