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

char * pathBusqueda;
char * atributo[4];
// atributo[0] = nombre
// atributo[1] = tipo
// atributo[2] = dueño
// atributo[3] = permisos
char * exe[11];

int main(int argc, char * argv[])
{
    pathBusqueda = strdup("/root");
    atributo[2] = strdup("root");
    buscadir(pathBusqueda);
    return 1;

}

void buscadir(char * path)
{
    DIR *directorio;
    int encontrado;
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
        // Se abre el archivo con stat
        stat(ptrdir->d_name, &archivo);
        //printf("Abierto archivo %s\n", ptrdir->d_name);

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
                case S_IFREG:  if( (strcmp(atributo[1], "r")) != 0) encontrado = 0; break;
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

        // Se obtiene la ruta absoluta del archivo
        strlcpy(resultado, cwd, sizeof(resultado));
        strlcat(resultado, "/", sizeof(resultado));
        strlcat(resultado, ptrdir->d_name, sizeof(resultado));

        // Si se encuentra el archivo, se imprime en pantalla su ruta absoluta
        if( encontrado && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
            printf("%s\n", resultado);

        // Si es un directorio, se lanza una busqueda recursiva dentro de el.
        if( ((archivo.st_mode & S_IFMT)==S_IFDIR) && (strcmp(ptrdir->d_name, ".") != 0) && (strcmp(ptrdir->d_name, "..") != 0) )
            buscadir(resultado);

    }
    // Se cierra el directorio porque si no es pecado y el mundo explota
    closedir(directorio);


}

