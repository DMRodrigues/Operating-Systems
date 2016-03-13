#ifndef __BUFFER_H_
#define __BUFFER_H_

struct indFicheiro {
	int indice;
	char *nome;
	int size;
	struct indFicheiro *next;
};
typedef struct indFicheiro *liga;

struct ptr {
	int tamanho; // tamanho do buffer
	int usados; // pa controlar se ainda ha coisas pa ler no buffer
	liga first; // nunca muda o primeiro elemento da lista, indice: 0
	liga head;
	liga tail;
};
typedef struct ptr *lista;


lista listaInit(int);

liga NEW(int, liga);

void listaInicializar(lista, int);

void listaInserir(lista, char*, int);

void listaRetirar(lista, char**);

int getTemDados(lista);

void printAll(lista);

void listaRemoveAll(lista);

#endif
