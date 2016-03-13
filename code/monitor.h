#ifndef __MONITOR_H_
#define __MONITOR_H_

/* por definicao escritor 1 filho, leitor 2 filho */
#define FILHOS 2
#define ESCRITOR 0
#define LEITOR 1

const char *filho_nome[FILHOS] = { "escritor",
									"leitor"
									};


/* para falar com o filho usamos fd 3 */
#define PIPE_FILHO 3

#define SEPARAR "\n"

/* mensagem inicial */
#define MSG1 "##############################################################################\n"
#define MSG1_LEN 80

#define MSG2 "\n"
#define MSG2_LEN 2

#define MSG3 "                        Olá, bem-vindo ao monitor!\n"
#define MSG3_LEN 53



#define MSG4 "                                                                         \x1B[37mv8.3\x1b[0m\n"
#define MSG4_LEN 87


int ficheiroOk(char*, int);
int comandoOk(char*, int, char *, int);

#endif
