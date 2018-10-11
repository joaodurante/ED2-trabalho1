/***************************************************************************************************
 * @title TrabalhoED2																			   *
 * @version 1.0																					   *
 * @authors Guilherme Krambeck, Jo�o Pedro Durante												   *
 ***************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

/*
#if defined(_WIN64)
    #include <conio.h>
    #define IS_WINDOWS 1
#elif defined(_WIN32)
    #include <conio.h>
    #define IS_WINDOWS 1
#else
    #include <curses.h>
    #define IS_WINDOWS 0
#endif
*/

#define BIBLIOTECA_REGISTER_LENGTH 119
#define SEPARADOR "#"
#define SINAL "*"
#define REGISTERS_OFFSET 4
#define ARCHIVE_FILENAME "archive.bin"
#define BIBLIOTECA_FILENAME "biblioteca.bin"
#define REMOVE_FILENAME "remove.bin"
#define TEMP_FILE_FILENAME "temp.bin"

struct book {
    char isbn[14];
    char titulo[50];
    char autor[50];
    char ano[5];
};
typedef struct book Book;

/**
 * Funcao que insere o 0 como indice de registros no arquivo
 *
 * @param file arquivo principal
 */
void initIndex(FILE *file);

/**
 * Inicia a lista (-1) de espacos disponiveis
 * @param file arquivo principal
 */
void initList(FILE *file);

/**
 * Funcao que retorna a quantidade de registros
 *
 * @param file arquivo principal
 * @return quantidade de registros
 */
int getIndex(FILE *file);

/**
 * Informa o numero de registros em um arquivo
 *
 * @param file arquivo principal
 * @return numero de registros do arquivo
 */
int getRegistryQuantity(FILE *file);

/**
 * Informa o numero de registros que registros que serao removidos
 *
 * @param remove arquivo de remoçao principal
 * @return numero de registros que serao removidos
 */
int getRemoveQuantity(FILE *remove);

/**
 * Responsável pela exibicao e distribuicao das operacoes
 */
void menu(Book *vetor, char removeVetor[][14]);

/**
 * Insercao de novos registros no arquivo
 *
 * @param vetor vetor com os dados carregados do arquivo biblioteca.bin
 * @param file arquivo principal
 */
void insert(Book *vetor, FILE *file);

/**
 * Incrementa o numero de registros lidos do arquivo biblioteca.bin
 * @param file arquivo principal
 * @param numberOfRegistersUsed
 */
void incrementNumberOfRegistersUsed(FILE *file, int numberOfRegistersUsed);

/**
 * Decrementa o numero de registros lidos do arquivo biblioteca.bin
 * @param file arquivo principal
 */
void decreaseNumberOfRegistersUsed(FILE *file);

/**
 * Funcao que faz o dump do arquivo
 */
void dumpFile(FILE* file);

/**
 * Funcao que insere o registro na variavel *string
 *
 * @param file arquivo principal
 * @param string arquivo que receberá o registro
 */
void selectRegister(FILE *file, char *string,  int bytes);

/**
 * Retorna o tamanho do registro
 *
 * @param file arquivo principal
 * @return tamanho do registro
 */
int getRegisterSize(FILE *file);

/**
 * Efetua a limpeza da tela
 */
void clearScreen();

/**
 * Aguarda o usuário pressionar qualquer tecla para
 * continuar a execução do programa
 */
void waitUserResponse();

/**
 * Inicia a lista de espaços disponiveis
 * @param file arquivo principal
 * @param vetor com os isbn que serao removidos
 */
void deleteRegister(FILE *file, char removeVetor[][14]);

/**
 * Retorna o ultimo elemento da lista de registros vazios
 * @param file arquivo principal
 * @return posicao do ultimo registro da lista de disponiveis
 */
int returnLastEmpty(FILE *file);

/**
 * Atualiza o penultimo elemento da lista de vazios para
 * apontar para o ultimo
 * @param file arquivo principal
 * @param posiçao do ultimo elemento
 */
void updateLastEmpty(FILE *file, int currentPosition);

/**
 * Cria um novo registro vazio resultado da insercao de um registro menor
 * que o espaco disponvivel
 * @param file arquivo principal
 * @param regSize (espaco vazio - espaco utilizado do registro inserido)
 * @param next offset do proximo elemento da lista de espacos vazios
 */
void createBlankRegister(FILE *file, int regSize, int next);

/**
 * Faz a compactacao do arquivo, demovendo os espacos vazios
 * @param file arquivo principal
 */
void compact(FILE* file);

/**
 * Verifica o status de um registro (se esta vazio ou nao)
 * @param string conteudo do registro
 * @return true or false
 */
int checkStatus(const char* string);

/**
 * Printa os valores em hexadecimal
 * @param s string com valores do registro
 */
void printHex(const char* s);



int main() {
    FILE *file, *remove;
    file = fopen(BIBLIOTECA_FILENAME, "rb");
    remove = fopen(REMOVE_FILENAME, "rb");
    if(!file || !remove){
        printf("Cannot open files\n");
        return 0;
    }

    int registerQuantity = getRegistryQuantity(file);
    Book vetor[registerQuantity];

    for(int i=0;i<registerQuantity;i++){
        fread(&vetor[i], sizeof(Book), 1, file);
    }
    fclose(file);
    
    int removeQuantity = getRemoveQuantity(remove) + 1;
    char removeVetor[removeQuantity][14];
    for(int i=0; i<removeQuantity - 1; i++){
    	fread(removeVetor[i], sizeof(char), 14, remove);
	}
	removeVetor[removeQuantity][0] = '\0'; //marcador de fim de array

    menu(vetor, removeVetor);
    printf("\n");
}

int getRegistryQuantity(FILE *file){
    fseek(file, 0L, SEEK_END);
    int registerQuantity = ftell(file) / BIBLIOTECA_REGISTER_LENGTH;
    rewind(file);
    return registerQuantity;
}

int getRemoveQuantity(FILE *remove){
	fseek(remove, 0, SEEK_END);
	int removeQuantity = ftell(remove) / 14; //14bytes = char[14]
	rewind(remove);
	return removeQuantity;
}

void initIndex(FILE *file){
	int index;
	rewind(file);
	if(!fread(&index, sizeof(int), 1, file)){
		index = 0;
		rewind(file);
		fwrite(&index, sizeof(int), 1, file);
	}
}

void initList(FILE *file){
	int head;
	
	fseek(file, 4, SEEK_SET);
	if(!fread(&head, sizeof(int), 1, file)){
		//fseek(file, -4, SEEK_CUR);
		head = -1;
		fwrite(&head, sizeof(int), 1, file);
	}
}

void menu(Book *vetor, char removeVetor[][14]){
    FILE *file;
    file = fopen(ARCHIVE_FILENAME, "r+b");
    if(!file){
        file = fopen(ARCHIVE_FILENAME, "w+b");
        initIndex(file);
        initList(file);
    }
    int resp;

    while(resp != 0){
      rewind(file);
      clearScreen();
      //printf("%s",BIBLIOTECA_FILENAME);
      printf("Menu");
      printf("\n1.Insercao");
      printf("\n2.Remocao");
      printf("\n3.Compactacao");
      printf("\n4.Dump Arquivo");
      printf("\n0.Sair");
      printf("\nEscolha: ");
      int resp;
      scanf("%d", &resp);
      switch(resp){
          case 1:
              insert(vetor, file);
              break;
          case 2:
              deleteRegister(file, removeVetor);
              break;
          case 3:
          	  compact(file);
              break;
          case 4:
              dumpFile(file);
              break;
          case 0: return;
              break;
          default: printf("\nEscolha Invalida");
      }
	      waitUserResponse();
    }
    fclose(file);
}

void insert(Book *vetor, FILE *file){
    int size, index, length, current, previous, next;

    rewind(file);
    index = getIndex(file);
    //Inserir no arquivo
    char buffer[200];

	length = strlen(vetor[index].isbn);
    length += strlen(vetor[index].titulo);
    length += strlen(vetor[index].autor);
    length += strlen(vetor[index].ano);
    length += 3; //###

	//Tratamento para encontrar um possivel espaco vazio e armazenar os valores (previous, current e next) da lista de disponiveis
	fread(&current, sizeof(int), 1, file);
	previous = 4;
	if(current == -1)
		fseek(file, 0, SEEK_END);
	else{
		fseek(file, current, SEEK_SET);
		fread(&size, sizeof(int), 1, file);
		if(size == length || (size - length) > 8){
			fseek(file, 1, SEEK_CUR);
			fread(&next, sizeof(int), 1, file);
			fseek(file, -9, SEEK_CUR); //retorna 9 bytes tamanho(4b) + sinal(1b) + prox(4b)
		}
		else{
			while(((size - length) < 9 && size != length) && current != -1){
				previous = current;
				fseek(file, 1, SEEK_CUR);
				fread(&current, sizeof(int), 1, file);
				if(current != -1){
					fseek(file, current, SEEK_SET);
					fread(&size, sizeof(int), 1, file);
					fseek(file, 1, SEEK_CUR);
					fread(&next, sizeof(int), 1, file);
					fseek(file, -5, SEEK_CUR);
				}
			}
			if(current != -1)
				fseek(file, -4, SEEK_CUR);
			else
				fseek(file, 0, SEEK_END);
		}
	}
    
    //Tratamento para insercao do registro no espaco encontrado
    if (length > 3){
    	sprintf(buffer, "%s%s%s%s%s%s%s", vetor[index].isbn, SEPARADOR, vetor[index].titulo, SEPARADOR, vetor[index].autor, SEPARADOR, vetor[index].ano);
    	fwrite(&length, sizeof(int), 1, file);
    	fwrite(buffer, strlen(buffer), 1, file);
    	printf("%s",buffer);
    	if(current != -1){
			if(size-length != 0){
				createBlankRegister(file, (size-length), next);
				current += length + 4; //offset do novo espaco vazio
				if(previous != 4){
					fseek(file, previous+5, SEEK_SET);
					fwrite(&current, sizeof(int), 1, file);
				}else{
					fseek(file, previous, SEEK_SET);
					fwrite(&current, sizeof(int), 1, file);
				}
			}else{
				if(previous != 4){
					fseek(file, previous+5, SEEK_SET);
					fwrite(&next, sizeof(int), 1, file);
				}else{
					fseek(file, previous, SEEK_SET);
					fwrite(&next, sizeof(int), 1, file);
				}
			}
		}
    	incrementNumberOfRegistersUsed(file, index);
	}else{
		printf("\nTodos os registros do arquivo %s ja foram consumidos'", BIBLIOTECA_FILENAME);
	}
}

int getIndex(FILE *file){
    int index;
    rewind(file);
    if(!fread(&index, sizeof(int), 1, file)){
        return 0;
    }else
        return index;
}

void incrementNumberOfRegistersUsed(FILE *file, int index){
    index++;
    rewind(file);
    fwrite(&index, sizeof(int), 1, file);
}

void decreaseNumberOfRegistersUsed(FILE *file){
	int index = getIndex(file);
	index--;
	rewind(file);
	fwrite(&index, sizeof(int), 1, file);
}

void compact(FILE* file){
	FILE* tempFile = fopen(TEMP_FILE_FILENAME,"w+b");
	
	//Write index
	int index = getIndex(file);
	fwrite(&index, sizeof(int), 1, tempFile);
	fseek(file, 8, SEEK_SET);
	
	//Write head of list
	int head = -1;
	fseek(tempFile, 4, SEEK_SET);
	fwrite(&head, sizeof(int), 1, tempFile);
	
    char string[120], *pch;
    int regSize;
	//Write Registers
    regSize = getRegisterSize(file);
    selectRegister(file, string, regSize);
    while(regSize > 0){
    	if (!checkStatus(string)){
        	fwrite(&regSize, sizeof(int), 1, tempFile);
    		fwrite(string, strlen(string), 1, tempFile);
        }
        regSize = getRegisterSize(file);
        selectRegister(file, string, regSize);
    }
    
    //Deletes the older file and rename new file
    fclose(file);
    fclose(tempFile);
    remove(ARCHIVE_FILENAME);
    rename(TEMP_FILE_FILENAME, ARCHIVE_FILENAME);
    file = fopen(ARCHIVE_FILENAME,"r+b");
    printf("\nCompactacao efetuada com sucesso!");
}

void dumpFile(FILE* file){
  	clearScreen();
  	printf("REGISTROS:\n\n");
    char string[120], *pch, regSize;

    //fseek para pular o indice de registros
    fseek(file, 8, SEEK_SET);

    regSize = getRegisterSize(file);
    selectRegister(file, string, regSize);
    int i =0;
    while(regSize > 0){
        i++;
    	//fseek(file, 4, SEEK_CUR);
    	if (!checkStatus(string)){
    	    printf("### %do registro ###\n", i);
        	pch = strtok(string, "#");
        	while(pch != NULL){
                printHex(pch);
            	printf("\n%s\n\n", pch);
            	pch = strtok(NULL, "#");
        	}
        	printf("\n");
		}
        regSize = getRegisterSize(file);
        selectRegister(file, string, regSize);
    }
}

void printHex(const char* s){
    int i = 0;
    while(s[i] != '\0'){
        printf("%x ", s[i]);
        i++;
    }
}

void selectRegister(FILE *file, char *string, int bytes){
    if(bytes > 0){
    	fread(string, bytes, 1, file);
    	string[bytes] = '\0';
	}
}

int getRegisterSize(FILE *file){
  	int bytes;
  	
  	if(fread(&bytes, sizeof(int), 1, file))
  		return bytes;
    else
    	return 0;
}

void clearScreen(){
    /*
    printf("%d",IS_WINDOWS);
    if (IS_WINDOWS === 1){
        system("cls");
    }else{
        system("clear");
    }
    */
   system("cls");
   //system("clear");
}

void waitUserResponse(){
    printf("\nPressione qualquer tecla para continuar...");
    getch();
    //getchar();
    //system("read b");
}

int checkStatus(const char* string){
	if(string[0] == '*')
		return 1;
	else
		return 0;
}

 void deleteRegister(FILE *file, char removeVetor[][14]){
 	char isbn[14];
 	fseek(file, 8, SEEK_SET);
 	int currentPosition = ftell(file);
 	int regSize = getRegisterSize(file);
 	int i = 0, END = -1;
 	
 	while(regSize > 0){
 		fread(isbn, sizeof(char), 13, file);
 		while(removeVetor[i][0] != '\0'){
 			
 			if(!strcmp(isbn, removeVetor[i])){
 				fseek(file, -13, SEEK_CUR);
 				fwrite(&SINAL, sizeof(char), 1, file);
 				fwrite(&END, sizeof(int), 1, file);
 				updateLastEmpty(file, currentPosition);
 				decreaseNumberOfRegistersUsed(file);
 				i++;
 			}else{
 				i++;
 			}
 		}
 		fseek(file, currentPosition, SEEK_SET);
 		fseek(file, regSize + 4, SEEK_CUR); //avan�a para a posi�ao do proximo registro
 		currentPosition = ftell(file);
 		regSize = getRegisterSize(file);
 		i=0;
 	}
 	printf("\nnnRemocao efetuada com sucesso!");
 }

 int returnLastEmpty(FILE *file){
 	int current, next;
 	
 	fseek(file, 4, SEEK_SET);
 	fread(&next, sizeof(int), 1, file);
 	if(next == -1)
 		return -1;
 	
 	while(next != -1){
 		current = next;
 		//next = next + tamanho(4bytes) + sinal(1byte)
 		next += 5;
 		fseek(file, next, SEEK_SET);
 		fread(&next, sizeof(int), 1, file);
 	}
 	return current;
 }

 void updateLastEmpty(FILE *file, int currentPosition){
 	int last = returnLastEmpty(file);
 	last += 5; //tamanho(4bytes) e sinal(1byte)
 	
 	fseek(file, last, SEEK_SET);
 	fwrite(&currentPosition, sizeof(int), 1, file);
 }
 
void createBlankRegister(FILE *file, int regSize, int next){
	
	if(regSize > 8){
		regSize -= 4;
		fwrite(&regSize, sizeof(int), 1, file);
		fwrite(&SINAL, sizeof(char), 1, file);
		fwrite(&next, sizeof(int), 1, file);
	}
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
