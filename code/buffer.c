/******************************************************************************/
/*                                                                            */
/*                               Buffer circular                              */
/*                                                                            */
/*      +-----------------------------------------------+                     */
/*      |                                               |                     */
/*      |   +--------+     +--------+     +--------+    |                     */
/*      |   | indice |     | indice |     | indice |    |                     */
/*      |   |  nome  |     |  nome  |     |  nome  |    |                     */ 
/*      |      size  |     |  size  |     |  size  |    |                     */
/*      +-->|    --------->|    --------->|    ---------+                     */
/*          +--------+     +--------+     +--------+                          */
/*            ^               ^               ^                               */
/*            |               |               |                               */ 
/*          first           tail            head                              */
/* (sempre o primeiro)    (retorna)   (apaga antigo e adiciona)               */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/* Inicializa um novo buffer a NULL, sem dados excepto tamanho
 * devolve -1 em caso erro */
lista listaInit(int tamanho)
{
	int i;
	
	lista q = (lista)malloc(sizeof(struct ptr));
	if(q == NULL)
		return NULL; //erro no malloc
	
	q->tamanho = tamanho;
	q->usados = 0;
	q->first = NULL;
	q->head = NULL;
	q->tail = NULL;
	
	/* inicializar buffer */
	for(i = 0; i < tamanho; i++)
		listaInicializar(q, i);
	
	q->first = q->head;
	q->tail = q->head; // repor tail pa primeira posicao
	
	return q;
}

/* adicionar no incio os nos na lista ligada */
void listaInicializar(lista q, int qual)
{
	if(q->head == NULL) {
		q->head = NEW(qual, NULL);
		q->head->next = q->head;
		q->tail = q->head; //usar tail no incrementa estrutura
		return;
	}
	q->tail->next = NEW(qual, q->head);
	q->tail = q->tail->next;
}


/* criar um novo no na lista ligada
 * devolve -1 em caso erro */
liga NEW(int qual, liga a)
{
	liga x = (liga)malloc(sizeof(struct indFicheiro));
	if(x == NULL)
		return NULL; //erro no malloc
	
	x->indice = qual;
	x->nome = NULL; // na criacao esta vazio
	x->size = 0; // na criacao e zero
	x->next = a;
	return x;
}


/* adicionar um novo nome na lista, e se tiver apagar antigo
 * imprime mensagem em caso erro */
void listaInserir(lista q, char *text, int text_size)
{
	/* copiar do buffer para a memoria dinamica e espaco '\0' pois pode n ter */
	char* nome = (char *)malloc(sizeof(char) * (text_size + 1));
	if(nome == NULL) {
		perror("Nao consegui introduzir nome buffer!");
		return; // indiferente aqi pois n comparo
	}
	
	strcpy(nome, text); // adiciona no fim '\0'
	
	/* se la tiver nome entao apaga */
	if(q->head->nome != NULL)
		free(q->head->nome);
	
	/* actualizar esse no e avancar pa proxima */
	q->head->nome = nome;
	q->head->size = text_size + 1; // ja com '\0' para o retirar
	q->head = q->head->next;
	q->usados++;
}


/* retirar o proximo no a ler da lista, recebe um ponteiro pa string a copiar e actualiza
 * devolve NULL em caso erro */
void listaRetirar(lista q, char **informacao)
{
	*informacao = (char*)malloc(sizeof(char) * q->tail->size);
	if(informacao == NULL)
		return; // indiferente aqi pois n comparo
	
	strcpy(*informacao, q->tail->nome);
	
	q->tail = q->tail->next;
	q->usados--;
}


/* saber se o buffer ainda tem dados pa ler */
int getTemDados(lista q)
{
	/* retornar o numero de dados ainda pa ler no buffer */
	return q->usados;
}


/* imprimir todo o conteudo do buffer */
void printAll(lista q)
{
	int i;
	liga t = q->first;
	
	for(i = 0; i < q->tamanho; i++) {
		if(t == q->head) {
			if(t == q->tail)
				printf("Indice: %d  valor: %s   <-- head | tail\n", t->indice, t->nome);
			else
				printf("Indice: %d  valor: %s   <-- head\n", t->indice, t->nome);
		}
		else if(t == q->tail)
			printf("Indice: %d  valor: %s   <-- tail\n", t->indice, t->nome);
		else
			printf("Indice: %d  valor: %s\n", t->indice, t->nome);
		t = t->next;
	}
}


/* remover todos os nos, e respectivos nomes se tiver, da lista */
void listaRemoveAll(lista q)
{
	int i;
	liga t = NULL;
	
	for(i = 0; i < q->tamanho; i++) {
		t = q->first->next;
		
		if(q->first->nome != NULL) // se la tiver nome
			free(q->first->nome);
		
		free(q->first);
		q->first = t;
	}
}
