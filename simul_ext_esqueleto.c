#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,  char *nombreantiguo, char *nombrenuevo);
//int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,  EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     //...
     
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Bucle de tratamiento de comandos
     for (;;){
		do {
		printf (">> ");
		fflush(stdin);
		fgets(comando, LONGITUD_COMANDO, stdin);
		} while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
        if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
            }
            else if(strcmp(orden, "info") == 0){
                    LeeSuperBloque(&ext_superblock);
            }
            else if (strcmp(orden,"bytemaps") == 0) {
                    Printbytemaps(&ext_bytemaps);
            }
            else if (strcmp(orden,"rename") == 0) {
                    Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
            }
         // Escritura de metadatos en comandos rename, remove, copy     
/*         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0; */
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }
     }
}


//Funcion para comprobar el comando, "menu"
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){

    char length[strlen(strcomando)];
    strncpy(length, strcomando, strlen(strcomando)-1);
    length[strlen(strcomando)-1] = '\0';
    char *token = strtok(length, " ");
    //Analizar de uno en uno
    if(token != NULL){
        int i;
        for(i = 0; token != NULL; i++){

            switch(i){


                case 0:
                        strcpy(orden, token);
                        token = strtok(NULL," ");
                        break;

                case 1:
                        strcpy(argumento1, token);
                        token = strtok(NULL," ");
                        break;

                case 2:
                        strcpy(argumento2, token);
                        token = strtok(NULL," ");
                        break;

                default:
                        return 1;
            }
        }

    }else{
        return 1;
    }
    if(strcmp(orden,"info") == 0){
        return 0;
    }else if(strcmp(orden, "bytemaps") == 0){
        return 0;
    }else if(strcmp(orden, "dir") == 0){
        return 0;
    }else if(strcmp(orden, "rename") == 0){
        return !(strlen(argumento1) > 0 && strlen(argumento2) > 0);
    }
}


//Funciones completas

//Muestra la información del superbloque
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){

   printf("Mostrando informacion sobre el superbloque:\n\n");
   printf("Bloque: %d Bytes\n", psup->s_block_size);
   printf("Inodos particion: %d\n", psup->s_inodes_count);
    printf("Inodos libres: %d\n", psup->s_free_inodes_count);
   printf("Bloques particion: %d\n", psup->s_blocks_count);
    printf("Bloques libres: %d\n", psup->s_free_blocks_count);
   printf("Primer bloque de datos: %d\n\n", psup->s_first_data_block);

}


//Muestra el contenido del bytemap de inodos y los 25 primeros elementos del bytemap de bloques.
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    //Inodos
    printf("Inodos:");
    for(int i=0; i<MAX_INODOS; i++){
        printf("%u ",ext_bytemaps->bmap_inodos[i]);
    }
    
    //Bloques
    printf("\nBloques [0-25] :");
    
    for(int i=0; i<MAX_BLOQUES_PARTICION; i++){
        printf("%d ",ext_bytemaps->bmap_bloques[i]);
    }
    
    printf("\n");
}


//Lista todos los ficheros (excepto la entrada especial del directorio raíz).

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
   int inodos_count = 0;

   //recorre el directorio / i=1 evita el directorio raiz
   for(int i = 1; i < inodos_count + 1; i++){
        // imprime tamaño e inodo
        printf("%s ", directorio[i].dir_nfich);
        printf("Tamaño: %d", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero);
        printf("Inodo: %d", directorio[i].dir_inodo);
        //imprime los bloques
        printf("Bloques: ");
        for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++){
            if(inodos -> blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != 65535){
                printf("%d ", inodos -> blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
            }
         }
        printf("\n");
      }
   }
   




int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){

   int i = 0;
   while(i < MAX_FICHEROS){ //bucle que recorre todo el directorio

        if(directorio[i].dir_inodo != NULL_INODO){
            if(strcmp(nombre, directorio[i].dir_nfich) == 0){
                return 0; //si el inodo no es nulo y el nombre de fichero existe, devuelve 0
            }
        }
    i++;
    }
return 1; //si no se encuentra el fichero, devuelve 1.
}



//Cambia el nombre de un fichero en la entrada correspondiente

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){  
//Comprueba que el archivo a renombrar existe y que el nombre no está en uso
   if(BuscaFich(directorio, inodos, nombreantiguo) == 0){  
      if(BuscaFich(directorio, inodos, nombrenuevo) == 1) { 
        //Recorrera el fichero hasta encontrar el que coincide con el nombre a renombrar      
        for(int i = 0; i < MAX_FICHEROS; i++){
            if(strcmp(directorio[i].dir_nfich, nombreantiguo) == 1){
            //Una vez lo encuentra "return 1" lo renombra
               strcpy(directorio[i].dir_nfich, nombrenuevo);
               return 0;        
           }      
        }      
      } else {
         printf("ERROR: El nombre está en uso.\n");  //si el nuevo nombre ya pertenece a un fichero, devuelve un error
         return 1;
      }
   } else {
      printf("ERROR: No existe el archivo.\n");   //si no se encuentra el fichero a renombrar, devuelve un error
      return 1;
   }
}


//Elimina el fichero
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
   //Comprueba que el archivo existe, si es asi lo borra
   if(BuscaFich(directorio, inodos, nombre) == 0){
      return 0;
   } else {
    //Si la funcino BuscaFich devolviera cualquier otro numero distinto de 0, saldria error
      printf("ERROR: El fichero no existe.\n");
      return 1;
   }
}


//Copiar un fichero.

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
   if(BuscaFich(directorio, inodos, nombreorigen) == 0){ //comprueba que el archivo que se quiere copiar existe
      if(BuscaFich(directorio, inodos, nombredestino) == 1){ //comprueba que el nombre de destino no existe
         return 0;
      } else {
         printf("ERROR: Ese fichero ya existe y no se puede reemplazar.\n");
         return 0;
      }
   } else {
      printf("ERROR: No existe el fichero que desea copiar.\n");
      return 0;
   }
}
