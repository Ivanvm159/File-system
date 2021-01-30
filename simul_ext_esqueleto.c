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
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *bnombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
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
         ...
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }
     }
}

//Superblock information
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
    printf("Bloque %d Bytes\n", psup->s_block_size);
    printf("inodos particion = %d\n", psup->s_inodes_count);
    printf("inodos libres = %d\n", psup->s_free_inodes_count);
    printf("Bloques particion %d\n", psup->s_blocks_count);
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
	//Recorre el directorio
    	//i=1 evitando el directorio raiz
	for(int i=1; i<inodos_count+1; i++){
		printf("%s ",directorio[i].dir_nfich); //imprime nombre
		printf("Tamaño:%d", inodos->blq_inodos[directorio[i].dir_inodo].size_fichero); //imprime tamaño
		printf("Inodo:%d",directorio[i].dir_inodo); // imprime inodo
		printf("Bloques:"); //imprime bloques
		//bloques ocupados por cada fichero
		for(int j=0; j<MAX_NUMS_BLOQUE_INODO; j++){
			if( inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]!= 65535){
				printf("%d ",inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
			}
		}
		printf("\n");
	}
	printf("\n");
}

int ComprobarFichero(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
	//recorre el directorio
	int i = 0;
	while(i < MAX_FICHEROS){
		if(directorio[i].dir_inodo != NULL_INODO){ // si el inodo no es nulo
        		if(strcmp(nombre, directorio[i].dir_nfich) == 0){ //si existe el fichero
            		return 0; //la funcion devuelve 0
         		}
      		}
      	i++;
   	}
return 1; //si el inodo es nulo o no existe tal fichero, devuelve 1
}
//la funcion devolvera 0 si se ejecuta correctamente y renombra los archivos
//la funcion devolvera 1 si encuentra algun error en el fichero a renombrar o en el nombre nuevo

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){  
   	if(ComprobarFichero(directorio, inodos, nombreantiguo) == 0){           //busca el fichero a renombrar, si existe pasa a la siguiente comprobacion   
   		if(ComprobarFichero(directorio, inodos, nombrenuevo) == 1) {         //busca que el nombre nuevo no exista en ningun fichero ya existente      
        		for(int i = 0; i < MAX_FICHEROS; i++){                       //recorre el directorio       
        			if(strcmp(directorio[i].dir_nfich, nombreantiguo) == 0){         
               			strcpy(directorio[i].dir_nfich, nombrenuevo);         //renombra el fichero
               		return 0;        
           		}      
        	}      
      	} else {
        	 printf("ERROR: ya existe un fichero con ese nombre.\n");  //si el nuevo nombre ya pertenece a un fichero, devuelve un error
        	 return 1;
      	}
   	} else {
   	   printf("ERROR: no existe un fichero con ese nombre.\n");   //si no se encuentra el fichero a renombrar, devuelve un error
   	   return 1;
   	}
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
	//Print inodos
   	 printf("Inodos:");
   	 for(int i=0; i<MAX_INODOS; i++){
       	 printf("%u ",ext_bytemaps->bmap_inodos[i]);
	 }
    
    //Print Bloques
    printf("\nBloques [0-25] :");
    for(int i=0; i<MAX_BLOQUES_PARTICION; i++){
        printf("%d ",ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *bnombre,  FILE *fich){
	if(ExisteFich(directorio, inodos, bnombre) == 0){
      		//borrar
      		return 0;
   	} else {
      		printf("ERROR: El fichero no existe.\n");
     		 return 0;
   	}
}





