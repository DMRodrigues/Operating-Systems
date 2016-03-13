/******************************************************************************/
/*                                                                            */
/*             Leitor de ficheiros com metodo pthread_create()                */
/*                          Leitor com monitor                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*                    Leitor com buffer circular integrado                    */
/*                                                                            */
/******************************************************************************/

#define _GNU_SOURCE   /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "geral.h"
#include "buffer.h"
#include "leitor.h"

/* temos 8 bits, minimo possivel C, equivalente char, tem valor 0 ou 1 unsigned */
static uint8_t leituraTerminar;


/* semaforo e mutex para controlar leituras do pipe */
sem_t temDadosLer, temEspacoEscrever;
pthread_mutex_t mutex;


/* buffer circular para usar COM semaforos */
static lista usar;

/**
 * FUNCAO PRINCIPAL
 * Funcao que faz a gestao das threads para cada leitor
 */
int main(void)
{
	long i;
	ssize_t size_lido;
	char buff[BUFSIZ]; // ler stdin
	
	int aux_size;
	char *token, *subtoken; // strings a dividir
	char *saveptr1, *saveptr2; // guardar strings originais
	
	void *status[THREADS]; // guardar valor saida dos filhos
	pthread_t threads[THREADS]; // informacao de todas threads
	
	leituraTerminar = LEITOR_NOK;
	
	errno = 0; // por variavel errno a zero para efectuar operacoes
	
	/* n temos nada pa ler inicio */
	if(sem_init(&temDadosLer, 0, 0) == -1)
		handle_error("Erro ao inicializar semaforo temDadosLer");
	
	/* temos todo espaco escrever inicio */
	if(sem_init(&temEspacoEscrever, 0, TAMANHO) == -1)
		handle_error("Erro ao inicializar semaforo temEspacoEscrever");
	
	/* criar um mutex pa exclusao mutua */
	if((errno = pthread_mutex_init(&mutex, NULL)) != 0)
		handle_error("Erro ao inicializar mutex");
	
	/* buffer com tamanho escolhido */
	usar = listaInit(TAMANHO);
	if(usar == NULL)
		handle_error("Erro ao inicializar buffer");
	
	/* Criar as threads e dizer a funcao e argumento */
	for(i = 0; i < THREADS; i++) {
		
		errno = pthread_create(&threads[i], NULL, funcaoLeitor, NULL);
		
		if(errno != 0)
			handle_error("Erro fazer thread create");
	}
	
	/* Ler do stdin os comandos */
	while((size_lido = read(STDIN_FILENO, buff, BUFSIZ)) > 0)
	{
		buff[size_lido-1] = '\0'; // adicionar fim string
		
		/* tem dados pa ler e n sabemos quantos */
		if(buff[0] != '\0')
		{
			/* separar mensagem por '\n' */
			token = strtok_r(buff, SEPARAR1, &saveptr1);
			
			while(token != NULL)
			{
				/* separar mensagem por espacos */
				subtoken = strtok_r(token, SEPARAR2, &saveptr2);
				
				while(subtoken != NULL)
				{
					aux_size = strlen(subtoken);
					
					/* TRATAR COMANDO ESPECIAL PRINT BUFFER */
					if((aux_size <= PRINT_LEN) && (strncmp(subtoken, PRINT, PRINT_LEN) == 0)) {
						
						if((errno = pthread_mutex_lock(&mutex)) != 0)
							perror("Erro fazer mutex lock no print");
						
						printAll(usar);
						
						if((errno = pthread_mutex_unlock(&mutex)) != 0)
							perror("Erro fazer mutex unlock no print");
					}
					
					else {
						if(sem_wait(&temEspacoEscrever) == -1)
							perror("Erro esperar semaforo main");
						
						if((errno = pthread_mutex_lock(&mutex)) != 0)
							perror("Erro tentar lock mutex");
						
						listaInserir(usar, subtoken, aux_size);
						
						if((errno = pthread_mutex_unlock(&mutex)) != 0)
							perror("Erro tentar unlock mutex");
						
						if(sem_post(&temDadosLer) == -1)
							perror("Erro assinalar semaforo main");
					}
					
					subtoken = strtok_r(NULL, SEPARAR2, &saveptr2);
				}
				
				token = strtok_r(NULL, SEPARAR1, &saveptr1);
			}
		}
	}
	
	if(size_lido == -1)
		perror("Erro na leitura monitor STDIN");
	
	
	/* pipe foi closed pelo monitor entao avisar todas threads filhas */
	if((errno = pthread_mutex_lock(&mutex)) != 0)
		perror("Erro tentar lock mutex");
	
	/* maneira simples mas funcional pa avisar todas threads para terminar */
	leituraTerminar = LEITOR_OK;
	for(i = 0; i < THREADS; i++) {
		if(sem_post(&temDadosLer) == -1)
			handle_error("Erro assinalar semaforo para terminar threads");
			//printf("Erro assinalar semaforo para terminar %ld: %s\n", i, strerror(erro));
	}
	
	if((errno = pthread_mutex_unlock(&mutex)) != 0)
		perror("Erro tentar unlock mutex");
	
	
	/* Esperar por todas as threads e ler valor saida */
	for(i = 0; i < THREADS; i++) {
		errno = pthread_join(threads[i], &status[i]);
		
		if(errno != 0)
			perror("Erro fazer thread join");
	}
	
	
	/* ja nao precisamos */
	if(sem_destroy(&temDadosLer) == -1)
		perror("Nao destrudio temDadosLer");
	if(sem_destroy(&temEspacoEscrever) == -1)
		perror("Nao destrudio temEspacoEscrever");
	if((errno =pthread_mutex_destroy(&mutex)) != 0)
		perror("Erro detruir mutex");
	
	/* eliminar buffer */
	listaRemoveAll(usar);
	free(usar);
	
	/* Imprimir valor devolvido de cada thread */
	for(i = 0; i < THREADS; i++) {
		if((long)status[i] == EXIT_SUCCESS)
			printf("Thread leitor %ld com sucesso retornou: %ld\n", i, (long)status[i]);
		else
			printf("Thread leitor %ld com erros retornou: %ld\n", i, (long)status[i]);
	}
	
	exit(EXIT_SUCCESS);
}


/**
 * Funcao funcaoLeitor
 * Verifica ficheiro, retorna: 0 sucesso, -1 erro
 */
void *funcaoLeitor()
{
	char control;
	int linha, estado;
	ssize_t file, state;
	char buffer[CADEIA_SIZE + 1]; // buffer para leituras de 10 caracteres
	
	char *erro_msg = NULL;
	char *fileName = NULL;
	
	for(;;) {
		control = BAD_CHAR; // repor control para reler
		
		errno = 0; // por variavel errno a zero para efectuar operacoes
		
		// ler buffer ou bloquear
		if(sem_wait(&temDadosLer) == -1)
			perror("Erro ler semaforo numa thread");
		
		if((errno = pthread_mutex_lock(&mutex)) != 0)
			perror("Erro lock mutex thread");
		
		/* antes de fechar lock testar sair do ciclo e entao terminar */
		if(leituraTerminar && (getTemDados(usar) == 0))
			break;
		
		
		/* obter o ponteiro para o nome ficheiro atraves do fileName */
		listaRetirar(usar, &fileName);
		if(fileName == NULL) {
			perror("Erro retirar nome buffer");
			continue; // erro retirar nome buffer passar frente
		}
		
		
		if(sem_post(&temEspacoEscrever) == -1)
			perror("Erro assinalar semaforo thread");
		
		if((errno = pthread_mutex_unlock(&mutex)) != 0)
			perror("Erro unlock mutex thread");
		
		
		file = open(fileName, open_mode);
		
		if(file == -1) {
			printf("Erro abrir %s: %s\n", fileName, strerror(errno));
			close(file);
			free(fileName);
			continue;
		}
		
		/* executar lock file pa ler */
		if(flock(file, read_mode) == -1) {
			/* verificar se o ficheiro tem lock de um writer pois flag LOCK_NB activa */
			if(errno == EWOULDBLOCK) {
				printf("Leitor: esperar para ler %s\n", fileName);
				
				if(flock(file, LOCK_SH) == -1) { // com lock bloquante
					if(asprintf(&erro_msg, "Erro lock %s", fileName) == -1)
						perror("Mensagem erro nao impressa, a sair! :(");
					exit_thread(file, erro_msg, fileName);
				}
			}
			else {// foi outro erro
				if(asprintf(&erro_msg, "Outro erro lock %s", fileName) == -1)
					perror("Mensagem erro nao impressa, a sair! :(");
				exit_thread(file, erro_msg, fileName);
			}
		}
		
		linha = 1; // linha usada como nr linha
		estado = OK; // inicio ficheiro como aceite
		
		/* ler 10 caracteres de cada vez ate acabar ou forcar a leitura de 1024 linhas */
		while( (state = read(file, buffer, CADEIA_SIZE)) || (linha <= LINHAS) )
		{
			if(state == -1) { // nao conseguimos ler nada, erro
				printf("%s não verificado!\n", fileName);
				estado=NOK;
				break;
			}
			
			/* n faz sentido continuar a ler */
			else if((state == 0) || (linha > LINHAS)) {
				printf("%s com erros\n", fileName);
				estado=NOK;
				break;
			}
			
			/* verificar primeira letra e SO na primeira vez igual 0 */
			else if(control == BAD_CHAR) {
				control = verPrimeiro(buffer[0], fileName); // ver o primeiro char
				if(control == BAD_CHAR) { // erro no primeiro char
					printf("%s com erro na linha 1: %s", fileName, buffer);
					if(buffer[CADEIA_SIZE - 1] != '\n') // se n tinha por
						printf("\n");
					estado=NOK;
					break;
				}
			}
			
			/* verificar resto das frases tendo conta 1 char */
			if(linhaOk(control, buffer) == BAD_STRING) {
				printf("%s com erros na linha %d: %s", fileName, linha, buffer);
				if(buffer[CADEIA_SIZE - 1] != '\n') // se n tinha por
					printf("\n");
				estado=NOK;
				break;
			}
			
			linha++; // mudar linha
		}
		
		/* executar unlock file depois de ler */
		if(flock(file, LOCK_UN) == -1) {
			if(asprintf(&erro_msg, "Erro unlock %s", fileName) == -1)
				perror("Mensagem erro nao impressa, a sair! :(");
			exit_thread(file, erro_msg, fileName);
		}
		
		if(close(file) == -1) {
			if(asprintf(&erro_msg, "Erro ao fechar %s", fileName) == -1)
				perror("Mensagem erro nao impressa, a sair! :(");
			exit_thread(file, erro_msg, fileName);
		}
		
		if(estado == OK) // ficheiro aceite
			printf("%s OK\n", fileName);
		
		free(fileName); // necessario fazer free do nome pa ler proximo ou sair
		
		//sleep(1); // so na brincadeira
	}
	
	if((errno = pthread_mutex_unlock(&mutex)) != 0)
		perror("Erro unlock mutex thread");
	
	if(erro_msg != NULL)
		free(erro_msg);
	
	pthread_exit((void *) EXIT_SUCCESS);
}



/**
 * FUNCAO linhaOk
 * Funcao que compara a linha lida com a linha correcta, retorna: 0 sucesso, BAD_STRING erro
 */
int linhaOk(char este, char* verificar) {
	/* usar conversao char pa inteiro onde
	 * a = 97
	 * ...
	 * j = 106 */
	int indice = este - CHAR_A;
	
	if(strncmp(verificar, cadeiax[indice], CADEIA_SIZE) != 0)
		return BAD_STRING;
	
	return EXIT_SUCCESS; // string certa e continuar leitura
}


/**
 * FUNCAO verPrimeiro
 * Funcao que ve o primeiro char de uma linha, retorna: char
 */
char verPrimeiro(char ver, char *fich_nome) {
	if((ver >= FIRST_CHAR) && (ver <= LAST_CHAR))
		return ver;
	else { // imprimir mensagem de erro com o char desconhecido
		//printf("Erro %s: caracter %c desconhecido\n", fich_nome, ver);
		return BAD_CHAR;
	}
}


/**
 * Funcao exit_thread
 * Recebe um file, uma string para imprimir, e um delete(filename) para apagar
 * Termina essa thread com FAILURE
 */
void exit_thread(ssize_t file, char* msg, char* delete) {
	perror(msg);
	free(msg);
	free(delete);
	close(file);
	pthread_exit((void *) FAILURE);
}
