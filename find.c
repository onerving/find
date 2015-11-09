#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>

void buscadir(char * path);
void evalParams(int * numarg, char ** arg);

char * pathBusqueda;
char * atributo[4];
// atributo[0] = nombre
// atributo[1] = tipo
// atributo[2] = dueNo
// atributo[3] = permisos
char * exe[11];
int  nexe = 0;

int main(int argc, char * argv[])
{
    evalParams(&argc, &argv[0]);
    buscadir(pathBusqueda);
    return 0;
}

//Funcion que separa los argumentos pasados por el usuario en dos arreglos, parametros y exec
void evalParams(int * numarg, char ** ptrarg)
{
    if(*numarg == 1)
    {
        printf("Uso: find [-ntdp] ruta [-exec comando [argumentos]]\n");
        printf("Donde: ");
        printf("-n: Nombre | ");
        printf("-t: Tipo | ");
        printf("-d: Duenio | ");
        printf("-p: Permisos (octal) | ");
        printf("-exec: Comando de shell con parametros\n");
        exit(1);
    }

    int i, j = 0;
    // Bandera que indica si se estan leyendo parametros de exec
    int scanexe = 0;

    for(i = 1; i<*numarg ; i++)
    {
        // Se mueve el apuntador al siguiente elemento de los args
        ptrarg++;
        // Si se encuentra una bandera y no estamos leyendo parametros de exec
        if( ((*ptrarg)[0] == '-') && !scanexe )
        {
            // Se verifica si hay otro argumento despues.
            if(++i<*numarg)
            {
                // Si se detecta alguna bandera, se registra su valor
                // en el atributo correspondiente
                if(!strcmp(*ptrarg, "-n"))
                    atributo[0] = *(++ptrarg);
                else if(!strcmp(*ptrarg, "-t"))
                    atributo[1] = *(++ptrarg);
                else if(!strcmp(*ptrarg, "-d"))
                    atributo[2] = *(++ptrarg);
                else if(!strcmp(*ptrarg, "-p"))
                    atributo[3] = *(++ptrarg);
                // Pero exec se guarda en su propio arreglo
                else if(!strcmp(*ptrarg, "-exec"))
                {
                    scanexe = 1;
                    nexe    = 0;
                    j = 0;  i--;
                }
                else
                {
                    printf("Bandera \"%s\" no reconocida.\n", *ptrarg);
                    exit(1);
                }

            }
            else
            {
                printf("Error: bandera %s sin valor presente\n", *ptrarg);
                exit(1);
            }
        }
        // Si se estan leyendo elementos de exec
        else if(scanexe)
        {
            // Se aniade al arreglo dedicado
            exe[j++] = *ptrarg;
            nexe++;
        }
        // Si es el primer path que se encuentra
        else if( pathBusqueda == NULL )
            pathBusqueda = *(ptrarg);
        // No se puede tener mas de un argumento de busqueda
        else
        {
            printf("No se puede buscar en mas de un directorio\n");
            exit(1);
        }

    }
    // Si en todos los argumentos no se especifico un path de busqueda
    if(pathBusqueda == NULL)
    {
        printf("Por favor introduzca una ruta para su busqueda\n");
        exit(1);
    }
}
//Funcion que utiliza los atributos obtenidos para encontrar archivos recursivamente desde la ruta requerida
void buscadir(char * path)
{
    // Para obtener la ruta donde se encuentra el archivo
    char cwd[1024];
    // Para obtener la ruta del archivo a comparar
    char resultado[2048];
    // Para abrir los directorios de busqueda
    DIR *directorio;
    struct dirent *ptrdir;
    // Para abrir el archivo a comparar
    struct stat archivo;
    int descriptor;
    // Banderas de error al hacer stat
    int status  = 0;
    int fstatus = 0;
    // Para obtener informacion de usuario dueNo del archivo
    struct passwd *pwd;
    // Para obtener los permisos del archivo
    char tempermisos[7];
    char *permisos;
    // Para manejar el comando que se usara en execvp
    char *comando;
    // Bandera que indica si el archivo coincide con nuestros parametros de busqueda
    int encontrado;

    // Nos movemos al directorio a escanear y obtenemos su path
    chdir(path);
    getcwd(cwd, sizeof(cwd));
    //printf("Directorio actual (cwd): %s\n", cwd);

    // Se abre el directorio
    if((directorio = opendir(cwd)) == NULL)
        {
            printf("No se pudo abrir el directorio %s\n", pathBusqueda);
            exit(1);
        }

    // Se itera a traves de todos los archivos en el directorio
    while( (ptrdir = readdir(directorio)) != NULL)
    {
        encontrado = 1;
        // Se obtiene la ruta absoluta del archivo uniendo:
        // El directorio donde estamos buscando
        strlcpy(resultado, cwd, sizeof(resultado));
        // Se anade una diagonal si no estamos en la raiz
        if(strcmp(cwd, "/")!=0)
            strlcat(resultado, "/", sizeof(resultado));
        // Se anade el nombre del archivo
        strlcat(resultado, ptrdir->d_name, sizeof(resultado));

        // Se abre el archivo con stat
        status = stat(resultado, &archivo);
        // Si no se pudo hacer stat, intentamos fstat
        if(status)
        {
            descriptor = open(resultado, O_RDONLY);
            fstatus = fstat(descriptor, &archivo);
            if(fstatus)
                lstat(resultado, &archivo);
        }
        //printf("Abierto archivo %s\n", resultado);

        // Se verifica que se cumplan los parametros especificados
        // Nombre, bandera -n
        if(atributo[0] != NULL)
        {
            // Si el nombre del archivo y nombre requerido no son iguales, se marca como no encontrado
            if(strcmp(ptrdir->d_name, atributo[0]) != 0)
                encontrado = 0;
        }
        // Tipo, bandera -t
        if(atributo[1] != NULL)
        {
            // Si el tipo del archivo y el tipo requerido no coinciden, se marca como no encontrado
            switch (archivo.st_mode & S_IFMT) {
                case S_IFBLK:  if( (strcmp(atributo[1], "b")) != 0) encontrado = 0; break;
                case S_IFCHR:  if( (strcmp(atributo[1], "c")) != 0) encontrado = 0; break;
                case S_IFDIR:  if( (strcmp(atributo[1], "d")) != 0) encontrado = 0; break;
                case S_IFIFO:  if( (strcmp(atributo[1], "f")) != 0) encontrado = 0; break;
                case S_IFLNK:  if( (strcmp(atributo[1], "l")) != 0) encontrado = 0; break;
                case S_IFREG:  if( (strcmp(atributo[1], "-")) != 0) encontrado = 0; break;
                case S_IFSOCK: if( (strcmp(atributo[1], "s")) != 0) encontrado = 0; break;
                default: break;
            }
        }
        // Dueno, bandera -d
        if(atributo[2] != NULL)
        {
            // Si el dueNo del archivo y el dueNo requerido no coinciden, se marca como no encontrado
            pwd = getpwuid(archivo.st_uid);
            if((strcmp(atributo[2], pwd->pw_name)) != 0)
                encontrado = 0;
        }
        // Permisos, badera -p
        if(atributo[3] != NULL)
        {
            // Se obtienen los permisos, se convierten a octal, y se pasan a una cadena
            snprintf(tempermisos, 7, "%o", archivo.st_mode);
            if(tempermisos[0]=='1')
                permisos = &tempermisos[3];
            else
                permisos = &tempermisos[2];
            //printf("Cadena permisos: %s\n", permisos);
            // Si los permisos del archivo y los requeridos no coinciden, se marca como no encontrado
            if((strcmp(permisos, atributo[3])) != 0)
                encontrado = 0;
        }


        // Si se encuentra el archivo (y no son los enlaces '.' y '..')
        if( encontrado && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
        {
            // Se imprime en pantalla la ruta absoluta
            printf("%s\n", resultado);
            // Si tenemos un comando a ejecutar
            if(nexe)
            {
                if(fork() == 0)
                {
                    // Se separa el comando para pasarlo como primer parametro
                    comando = strdup(exe[0]);
                    // Se prepara el arreglo de argumentos para el segundo parametro
                    exe[nexe++] = strdup(resultado);
                    exe[nexe++] = NULL;
                    // Se ejecuta
                    execvp(comando,exe);
                }
                else
                {
                    // El padre espera al hijo
                    wait(&fstatus);
                }
            }
        }

        // Si es un directorio, se lanza una busqueda recursiva dentro de El
        if( ((archivo.st_mode & S_IFMT)==S_IFDIR) && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
            buscadir(resultado);

        // Si era un archivo abierto, se cierra su descriptor
        if(status == 0)
            close(descriptor);

    }
    // Se cierra el directorio porque si no es pecado y el mundo explota
    closedir(directorio);
}

