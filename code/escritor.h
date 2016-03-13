#ifndef __ESCRITOR_H_
#define __ESCRITOR_H_

#define THREADS 3


#define DUMB 1 // qual incremento pa char no erro
#define INDICE 0 // qual o indice a incrementar


#define ESCRITOR_NOK 0
#define ESCRITOR_OK 1


/* definir uma funcao random; retorna: entre 0 e max-1*/
#define meuRandom(max) rand()%max


/* expressões condicionais
<expr1> ? <expr2> : <expr3>
Se <expr1> for verdadeiro, valor da expressão é <expr2>. Se <expr1> for falso, valor da expressão é <expr3> */
#define trocar(result) \
	do { result = result ? 0 : 1; } while (0)

/* o modo em uso no SIGUSR1 */
#define MODOLOCK "LOCK -> "
#define STRLEN_MODOLOCK 8

/* o modo em uso no SIGUSR2 */
#define MODOESCR "CERTO -> "
#define STRLEN_MODOESCR 9


/* muda cor terminal para vermelho escreve 'DESLIGADO' e repoe cor */
#define DESLIGADO "\x1B[31mDESLIGADO\x1B[0m\n"
#define STRLEN_DESLIGADO 19

/* muda cor terminal para verde escreve 'LIGADO' e repoe cor */
#define LIGADO "\x1B[32mLIGADO\x1B[0m\n"
#define STRLEN_LIGADO 16


/* imprime a expressao ligado ou desligado conforme este esta true ou false */
#define expressao(este) \
	do { este ? write(STDOUT_FILENO, LIGADO, STRLEN_LIGADO) : write(STDOUT_FILENO, DESLIGADO, STRLEN_DESLIGADO); } while (0)


/* abrir para escrita */
const int open_mode = O_WRONLY;

/* user = read, write; group = read; others = read */
const mode_t my_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

/* lock exckusivo bloqueante */
const int write_mode = LOCK_EX;


void porSignal(struct sigaction *, void*, int);
void *funcaoEscritor();

void print_erro(char**);

void exit_thread(ssize_t, char*);

#endif
