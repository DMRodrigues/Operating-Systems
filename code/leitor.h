#ifndef __LEITOR_H_
#define __LEITOR_H_

#define THREADS 3

#define BAD_CHAR 0
#define BAD_STRING -1


/* tamanho do buffer e controlar semaforos */
#define TAMANHO 5


#define LEITOR_NOK 0
#define LEITOR_OK 1


#define CHAR_A 97
#define FIRST_CHAR 'a'
#define LAST_CHAR 'j'

#define SEPARAR1 "\n"
#define SEPARAR2 " "


/* modo de abertura para leitura */
const int open_mode = O_RDONLY;

/* lock partilha e n bloquante a primeira vez */
const int read_mode = LOCK_SH | LOCK_NB;

void *funcaoLeitor();
int linhaOk(char, char*);
char verPrimeiro(char, char*);
int ficheiroOk(char*, int);

void exit_thread(ssize_t, char*, char*);

#endif
