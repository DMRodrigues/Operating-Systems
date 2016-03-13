#ifndef __GERAL_H_
#define __GERAL_H_

/* Contem inforamação sobre os ficheiros, e o seu conteudo.
 * E ainda uma definicao de estado entre processos e tarefas, entre outras
 */


/* numero de erro enviar caso termine mal */
#define FAILURE -1


/* Imprimir a mensagem erro e terminar programa */
#define handle_error(msg) \
	do { perror(msg); exit(FAILURE); } while (0)


/* estado interno do programa entre threads */
#define OK 0
#define NOK -1


/* qual o comando pa sair e tamanho */
#define SAIR "sair"
#define SAIR_LEN 4


/* comandos para enviar signal */
#define USR1 "il"
#define USR2 "ie"
#define USR_LEN 2


/* o sinal de sair po escritor */
#define SAIR_ESCRITOR SIGTSTP


/* COMANDO especial que adiciona-mos para imprimir o buffer */
#define PRINT "print"
#define PRINT_LEN 5


/* definições variadas dos ficheiros */
#define LINHAS 1024
#define FILE_SIZE 10240


#define NUM_FILES 5

char *filex[NUM_FILES] = {"SO2014-0.txt",
							"SO2014-1.txt",
							"SO2014-2.txt",
							"SO2014-3.txt",
							"SO2014-4.txt"
							};


#define NUM_CADEIAS 10
#define CADEIA_SIZE 10

const char *cadeiax[NUM_CADEIAS] = {"aaaaaaaaa\n",
									"bbbbbbbbb\n",
									"ccccccccc\n",
									"ddddddddd\n",
									"eeeeeeeee\n",
									"fffffffff\n",
									"ggggggggg\n",
									"hhhhhhhhh\n",
									"iiiiiiiii\n",
									"jjjjjjjjj\n"
									};


#endif
