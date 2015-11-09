#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

void buscadir(char * path);
void evalParams(int * numarg, char ** arg);

char * pathBusqueda;
char * atributo[4];
// atributo[0] = nombre
// atributo[1] = tipo
// atributo[2] = dueño
// atributo[3] = permisos
char * exe[11];

int main(int argc, char * argv[])
{
    evalParams( &argc, &argv[0]);
    buscadir(pathBusqueda);
    return 0;

}

//Funcion que separa los argumentos pasados por el usuario en dos
//arreglos, parametros y exec
void evalParams(int * numarg, char ** ptrarg)
{
    if(*numarg == 1)
    {
        printf("Uso: find [-ntdp] ruta [-exec comando [argumentos]]\n");
        printf("Donde: ");
        printf("-n: Nombre | ");
        printf("-t: Tipo | ");
        printf("-d: Dueño | ");
        printf("-p: Permisos (octal)\n");
        exit(1);
    }

    int i, j=0;
    int scanexe = 0;

    for(i = 1; i<*numarg ; i++)
    {
        // Se mueve el apuntador al siguiente elemento de los args
        ptrarg++;
        // Si se encuentra una bandera
        if( ((*ptrarg)[0] == '-') && !scanexe )
        {
            // Se verifica si hay otro argumento después.
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
                else if(!strcmp(*ptrarg, "-exec"))
                {
                    scanexe = 1;
                    j = 0; i--;
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
        // Si antes hubo una bandera exe
        else if(scanexe)
        {
            exe[j++] = *ptrarg;
        }
        // Si no se habia encontrado otro path
        else if( pathBusqueda == NULL )
            pathBusqueda = *(ptrarg);
        // No se puede tener mas de un argumento de busqueda
        else
        {
            printf("No se puede buscar en mas de un directorio\n");
            exit(1);
        }

    }
    if( pathBusqueda == NULL)

    {
        printf("Por favor introduzca una ruta para su busqueda\n");
        exit(1);
    }

}
void buscadir(char * path)
{
    DIR *directorio;
    int encontrado;
    char tempermisos[7];
    char *permisos;
    char cwd[1024];
    char resultado[2048];
    struct passwd *pwd;
    struct dirent *ptrdir;
    struct stat archivo;

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
        // Se obtiene la ruta absoluta del archivo
        // El directorio donde se encuentra la busqueda
        strlcpy(resultado, cwd, sizeof(resultado));
        // Se anade una diagonal si no estamos en la raiz
        if(strcmp(cwd, "/")!=0)
            strlcat(resultado, "/", sizeof(resultado));
        // Se anade el nombre del archivo
        strlcat(resultado, ptrdir->d_name, sizeof(resultado));

        // Se abre el archivo con stat
        stat(resultado, &archivo);
        //printf("Abierto archivo %s\n", resultado);

        // Se verifica que se cumplan los parametros especificados
        // Nombre, bandera -n
        if(atributo[0] != NULL)
        {
            if(strcmp(ptrdir->d_name, atributo[0]) != 0)
                encontrado = 0;
        }
        // Tipo, bandera -t
        if(atributo[1] != NULL)
        {
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
            pwd = getpwuid(archivo.st_uid);
            if((strcmp(atributo[2], pwd->pw_name)) != 0)
                encontrado = 0;
        }
        // Permisos, badera -p
        if(atributo[3] != NULL)
        {
            // Se obtienen los permisos de duenio, grupo y otros mediante los macros
            //printf("Int permisos: %o\n", archivo.st_mode);
            // Se pasa a una cadena octal
            snprintf(tempermisos, 7, "%o", archivo.st_mode);
            if(tempermisos[0]=='1')
                permisos = &tempermisos[3];
            else
                permisos = &tempermisos[2];
            //printf("Cadena permisos: %s\n", permisos);
            // Se compara
            if((strcmp(permisos, atributo[3])) != 0)
                encontrado = 0;
        }


        // Si se encuentra el archivo, se imprime en pantalla su ruta absoluta
        if( encontrado && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
        {
            printf("%s", permisos);
            printf("%s\n", resultado);
        }

        // Si es un directorio, se lanza una busqueda recursiva dentro de el.
        if( ((archivo.st_mode & S_IFMT)==S_IFDIR) && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
            buscadir(resultado);

    }
    // Se cierra el directorio porque si no es pecado y el mundo explota
   closedir(directorio);


}

