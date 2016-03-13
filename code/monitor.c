/******************************************************************************/
/*                                                                            */
/*                         Instituto Superior Tecnico                         */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*                             Sitemas Operativos                             */
/*                                  2014/2015                                 */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*                            Monitor independente                            */
/*                                                        versão final (v8.3) */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>

#include "geral.h"
#include "monitor.h"


int main(void)
{
	int i, status;
	int pipefd[2]; // pipe pa comunicacao
	char buff[BUFSIZ]; // ler stdin
	ssize_t size_lido;
	
	char *token;
	int aux_size;
	uint8_t flag;
	char *saveptr1;
	
	pid_t filho[FILHOS]; // informacao de todos filhos
	int exitfilho[FILHOS]; // informacao como filho saiu
	
	errno = 0; // por variavel errno a zero para efectuar operacoes
	
	/* ********** criar processo filho escritor ********** */
	filho[ESCRITOR] = fork();
	if(filho[ESCRITOR] == -1) // Erro ao criar filho
		handle_error("Erro criar filho escritor");
	
	else if(filho[ESCRITOR] == 0) { // codigo do filho
		if(execl(filho_nome[ESCRITOR], filho_nome[ESCRITOR], NULL) == -1)
			perror("Erro executar filho escritor");
		_exit(FAILURE);
	}
	
	/* ********** criar pipe comunicacao ********** */
	if(pipe(pipefd) == -1)
		handle_error("Erro ao criar pipe");
	
	/* ********** criar filho leitor ********** */
	filho[LEITOR] = fork();
	if(filho[LEITOR] == -1)
		handle_error("Erro criar filho leitor");
	
	else if(filho[LEITOR] == 0) {
		
		/* input do filho sendo pipe */
		if(dup2(pipefd[0], STDIN_FILENO) == -1)
			handle_error("Erro tentar dup2 filho");
		
		if(close(pipefd[0]) == -1) // input do pipe ja duplicado
			perror("Erro tentar filho close pipe in");
		
		if(close(pipefd[1]) == -1) // output do pipe filho nao usa
			perror("Erro tentar filho close pipe out");
		
		if(execl(filho_nome[LEITOR], filho_nome[LEITOR], NULL) == -1)
			perror("Erro executar filho leitor");
		_exit(FAILURE);
	}
	else {
		/* output po filho sendo pipe */
		if(dup2(pipefd[1], PIPE_FILHO) == -1)
			handle_error("Erro tentar dup2 pai");
		
		if(close(pipefd[1]) == -1) // pai nao necessita disto
			perror("Erro tentar pai close pipe output");
	}
	
	/* ********** imprimir mensagem de entrada ********** */
	write(STDOUT_FILENO, MSG1, MSG1_LEN);
	write(STDOUT_FILENO, MSG2, MSG2_LEN);
	write(STDOUT_FILENO, MSG3, MSG3_LEN);
	write(STDOUT_FILENO, MSG4, MSG4_LEN);
	write(STDOUT_FILENO, MSG1, MSG1_LEN);
	
	sleep(1); // dar tempo aos filhos pa iniciarem
	
	flag = 1;
	
	/* Ler do stdin os comandos */
	while(flag && ((size_lido = read(STDIN_FILENO, buff, BUFSIZ)) > 0) )
	{
		buff[size_lido-1] = '\0'; // adicionar fim string
		
		/* TEORICAMENTE tudo para alem de "sair", "il" e "ie" e o nosso "print" e nome de ficheiros */
		if(buff[0] != '\0')
		{
			token = strtok_r(buff, SEPARAR, &saveptr1);
			
			while(token != NULL)
			{
				aux_size = strlen(token) + 1; // tamanho da linha lida com '\0'
				
				/* comando sair */
				if(comandoOk(token, aux_size, SAIR, SAIR_LEN) == OK) {
					write(STDOUT_FILENO, "\n", 1); // mudanca linha pa se ver diferenca
					flag=0;
					break;
				}
				
				/* signal SIGUSR1: inverter flocks */
				else if(comandoOk(token, aux_size, USR1, USR_LEN) == OK) {
					if(kill(filho[ESCRITOR], SIGUSR1) == -1)
						perror("NAO CONSEGUI ENVIAR SIGUSR1");
				}
				
				/* signal SIGUSR2: inverter escrita erronea */
				else if(comandoOk(token, aux_size, USR2, USR_LEN) == OK) {
					if(kill(filho[ESCRITOR], SIGUSR2) == -1)
						perror("NAO CONSEGUI ENVIAR SIGUSR2");
				}
				
				/* comunicamos com filho, enviamos print ou nome ficheiro */
				else {
					if(write(PIPE_FILHO, token, aux_size) != aux_size)
						perror("Nao consegui enviar nome ficheiro leitor!\n");
				}
				
				//sleep(1); // dar tempo propagar po leitor e escritor <<< necessario file input
				
				token = strtok_r(NULL, SEPARAR, &saveptr1);
			}
		}
	}
	
	/* erro na leitura STDIN erro no processamento */
	if(size_lido == -1)
		perror("Erro na leitura monitor STDIN");
	
	
	/* ********** avisar filhos pa terminar ********** */
	if(close(PIPE_FILHO) == -1)
		perror("NAO CONSEGUI FECHAR PIPE");
	
	if(kill(filho[ESCRITOR], SAIR_ESCRITOR) == -1)
		perror("NAO CONSEGUI ENVIAR TERMINAR ESCRITOR");
	
	
	/* ler valor saida filhos */
	for(i = 0; i < FILHOS; i++) {
		filho[i] = waitpid(filho[i], &status, 0);
		
		/* esperar os filhos por ordem para mapear */
		if(filho[i] == -1)
			perror("Erro ao ler wait filho");
		
		if(WIFEXITED(status)) {
			if(status == EXIT_SUCCESS)
				exitfilho[i] = (char)WEXITSTATUS(status);
			else
				exitfilho[i] = (char)WEXITSTATUS(status);
		}
		else // erro nao nosso
			exitfilho[i] = status;
	}
	
	/* imprimir valor saida filhos conforme o erro */
	for(i = 0; i < FILHOS; i++) {
		if(exitfilho[i] == EXIT_SUCCESS)
			printf("Filho %s com sucesso retornou: %d\n", filho_nome[i], exitfilho[i]);
		else if(exitfilho[i] == FAILURE)
			printf("Filho %s com erros retornou: %d\n", filho_nome[i], exitfilho[i]);
		else
			printf("FILHO %s TERMINOU ERRADAMENTE: %d\n", filho_nome[i], exitfilho[i]);
	}
	
	exit(EXIT_SUCCESS);
}

/**
 * FUNCAO lerComando
 * Funcao que compara a linha lida com um dos comandos
 * Retorna: OK sucesso, NOK erro
 */
int comandoOk(char* verificar, int size_test, char *este, int size) {
	if(size_test != size+1)
		return NOK;
	
	if(strncmp(verificar, este, size) == 0)
		return OK;
	
	return NOK; // nao e um comando
}
