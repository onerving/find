#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void evalParams(int * numarg, char ** arg);
void pruebas();

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
    pruebas();
    return(0);
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
        printf("-p: Permisos\n");
        exit(1);
    }

    int i, j=0;
    int scanexe = 0;

    for(i = 1; i<*numarg ; i++)
    {
        // Se mueve el apuntador al siguiente elemento de los args
        ptrarg++;
        // Si se encuentra una bandera
        if( (*ptrarg)[0] == '-' )
        {
            if(scanexe) scanexe = 0;
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

void pruebas()
{
    int i;
    printf("Atributos\n");
    for(i = 0; i < 4; i++)
    {
        switch(i)
        {
            case 0: printf("Nombre:"); break;
            case 1: printf("Tipo:"); break;
            case 2: printf("Dueño:"); break;
            case 3: printf("Permisos:"); break;
        }
        printf(" %s\n", atributo[i]);

    }
    printf("Ruta de la busqueda: %s\n", pathBusqueda);
    printf("Exec:\n");
    for(i = 0; i < 11; i++)
    {
        if(exe[i] == NULL) break;
        printf("\t%s\n", exe[i]);
    }


}





