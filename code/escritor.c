/******************************************************************************/
/*                                                                            */
/*            Escritor de ficheiros com metodo pthread_create()               */
/*                        Escritor com monitor                                */
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
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "geral.h"
#include "escritor.h"


/* temos 8 bits, minimo possivel C, equivalente char, tem valor 0 ou 1 unsigned */
static uint8_t escritaTipo, escritaLock, escritaTerminar;


/**
 * Funcao stopSignal
 * Funcao que trata signal SIGTSTP para terminar os escritores
 */
static void terminarSignal(int signum)
{
	escritaTerminar = ESCRITOR_OK;
}


/**
 * Funcao altLock
 * Funcao que trata signal SIGUSR1 inverte escrita com os locks
 */
static void altLock(int signum)
{
	trocar(escritaLock);
	write(STDOUT_FILENO, MODOLOCK, STRLEN_MODOLOCK);
	expressao(escritaLock);
}


/**
 * Funcao altEscrita
 * Funcao que trata signal SIGUSR2 inverte escrita com ou usem erros
 */
static void altEscrita(int signum)
{
	trocar(escritaTipo);
	write(STDOUT_FILENO, MODOESCR, STRLEN_MODOESCR);
	expressao(escritaTipo);
}



/**
 * FUNCAO PRINCIPAL
 * Funcao que faz a gestao das threads para cada escritor
 */
int main(void)
{
	long i;
	void *status[THREADS]; // guardar valor saida dos filhos
	pthread_t threads[THREADS]; // informacao de todas threads
	struct sigaction stp, usr1, usr2; // informacao sinais
	
	/* por nosso signal terminar */
	porSignal(&stp, terminarSignal, SAIR_ESCRITOR);
	if(sigaction(SAIR_ESCRITOR, &stp, NULL) == -1)
		handle_error("Por sinal SIGTSTP");
	
	/* por nosso signal alternar lock */
	porSignal(&usr1, altLock, SIGUSR1);
	if(sigaction(SIGUSR1, &usr1, NULL) == -1)
		handle_error("Por sinal SIGUSR1");
	
	/* por nosso signal alternar escrita erronea */
	porSignal(&usr2, altEscrita, SIGUSR2);
	if(sigaction(SIGUSR2, &usr2, NULL) == -1)
		handle_error("Por sinal SIGUSR2");
	
	/* iniciar random seed: evitar obter sempre mesmo nr */
	srandom((unsigned) time(NULL));
	
	escritaTipo = ESCRITOR_OK; /* temos lock e escrita certa */
	escritaLock = ESCRITOR_OK;
	escritaTerminar = ESCRITOR_NOK; /* nao temos estado terminar */
	
	
	/* criar as threads e dizer a funcao */
	for(i = 0; i < THREADS; i++) {
		
		errno = pthread_create(&threads[i], NULL, funcaoEscritor, NULL);
		
		if(errno != 0)
			handle_error("Erro fazer thread create");
	}
	
	
	/* esperar por todas as threads e ler valor saida */
	for(i = 0; i < THREADS; i++) {
		errno = pthread_join(threads[i], &status[i]);
		
		if(errno != 0)
			perror("Erro fazer thread join");
	}
	
	/* imprimir valor devolvido de cada thread */
	for(i = 0; i < THREADS; i++) {
		if((long)status[i] == EXIT_SUCCESS)
			printf("Thread escritor %ld com sucesso retornou: %ld\n", i, (long)status[i]);
		else
			printf("Thread escritor %ld com erros retornou: %ld\n", i, (long)status[i]);
	}
	
	return EXIT_SUCCESS;
}



/**
 * Funcao funcaoEscritor
 * Escreve ficheiro, retorna: 0 sucesso, -1 erro
 */
void *funcaoEscritor()
{
	ssize_t file;
	int ciclo, aux;
	char *fileName;
	char str_write[CADEIA_SIZE];
	uint8_t tipoMeu = escritaTipo; // buscar valor lock
	uint8_t lockMeu = escritaLock; // buscar o valor escrita
	
	char *erro_msg = NULL; // contem a mensagem erro
	
	for(;;) {
		if(tipoMeu != escritaTipo)
			tipoMeu = escritaTipo;
		
		if(lockMeu != escritaLock)
			lockMeu = escritaLock;
		
		errno = 0; // errno a zero para efectuar operacoes
		
		fileName = filex[meuRandom(NUM_FILES)];
		
		file = open(fileName, open_mode);
		
		/* interpretar erros do sistema pelo errno */
		if(file == -1) {
			if(asprintf(&erro_msg, "Erro abrir %s", fileName) == -1)
				perror("Mensagem erro nao impressa, a sair! :(");
			print_erro(&erro_msg); // operacoes com mensagem erro
			
			if(errno == ENOENT) { // nao existia ficheiro
				file = creat(fileName, my_mode);
				
				if(file == -1) {
					if(asprintf(&erro_msg, "Erro ao criar %s", fileName) == -1)
						perror("Mensagem erro nao impressa, a sair! :(");
					exit_thread(file, erro_msg);
				}
				//printf("%s criado com sucesso\n", fileName);
			}
			else { /* Todos os outros erros que possam haver */
				if(asprintf(&erro_msg, "Erro sistema abrir %s", fileName) == -1)
					perror("Mensagem erro nao impressa, a sair! :(");
				exit_thread(file, erro_msg);
			}
		}
		
		/* verificar se e escrita com ou sem lock */
		if(lockMeu) {
			/* executar lock file pa escrever */
			if(flock(file, write_mode) == -1) {
				if(asprintf(&erro_msg, "Erro lock %s", fileName) == -1)
					perror("Mensagem erro nao impressa, a sair! :(");
				exit_thread(file, erro_msg);
			}
		}
		
		/* restringir ficheiro ate onde nos escrevemos tirando o resto po lixo */
		if(truncate(fileName, FILE_SIZE) == -1) {
			if(asprintf(&erro_msg, "Erro truncar %s", fileName) == -1)
				perror("Mensagem erro nao impressa, a sair! :(");
			print_erro(&erro_msg); // operacoes com mensagem erro
		}
		
		/* escolher e guardar string a escrever */
		strcpy(str_write, cadeiax[meuRandom(NUM_CADEIAS)]);
		
		aux = 0; // contador para fazer linhas erradas alternadas
		
		for(ciclo = 0; ciclo < LINHAS; ciclo++) {
			
			/* confirmar escrita de 10 caracteres ou caso dar -1  erro */
			if(write(file, str_write, CADEIA_SIZE) != CADEIA_SIZE)
				exit_thread(file, "Erro sistema escrever");
			
			/* verificar se escrever com erro ou repor */
			if(!tipoMeu){
				if(aux) {
					str_write[INDICE] -= DUMB;
					aux = 0;
				}
				else {
					str_write[INDICE] += DUMB; // um simples erro
					aux++;
				}
			}
		}
		
		if(lockMeu) {
			/* executar unlock file depois de escrever */
			if(flock(file, LOCK_UN) == -1) {
				if(asprintf(&erro_msg, "Erro unlock %s", fileName) == -1)
					perror("Mensagem erro nao impressa, a sair! :(");
				exit_thread(file, erro_msg);
			}
		}
		
		if(close(file) == -1) {
			if(asprintf(&erro_msg, "Erro ao fechar %s", fileName) == -1)
				perror("Mensagem erro nao impressa, a sair! :(");
			exit_thread(file, erro_msg);
		}
		
		/* entao sair do ciclo e terminar */
		if(escritaTerminar)
			break;
	}
	
	if(erro_msg != NULL)
		free(erro_msg);
	
	pthread_exit((void *) EXIT_SUCCESS);
}




/**
 * Funcao porSignal
 * Inicializa o tipo de signal e funcao handler que quisermos
 */
void porSignal(struct sigaction *isto, void *handler, int signum) {
	isto->sa_handler = handler; // usar esta
	if(sigemptyset(&isto->sa_mask) == -1) // por todos sinais excluidos
		handle_error("Erro sigemptyset por sinais porSignal");
	if(sigaddset(&isto->sa_mask, signum) == -1) // adicionar o nosso sinal a mascara
		handle_error("Erro sigaddset por sinais porSignal");
	isto->sa_flags = 0; // ignorado
}



/**
 * Funcao print_erro
 * Recebe uma string para imprimir e procede a sua reposicao
 */
void print_erro(char** msg) {
	perror(*msg);
	free(*msg); // fazer free da mensagem
	*msg = NULL; // repor ponteiro pa mais operacoes
}


/**
 * Funcao exit_thread
 * Recebe um file e uma string para imprimir em caso erro
 * Termina essa thread com FAILURE
 */
void exit_thread(ssize_t file, char* msg) {
	perror(msg);
	free(msg);
	close(file);
	pthread_exit((void *) FAILURE);
}
