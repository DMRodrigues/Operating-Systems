# compiler flags:
#  -g flag para usar no DDD
#  -Wall compiler warnings
CC = gcc
CFLAGS = -g -Wall
PTHREAD = -pthread

all: leitor escritor monitor


escritor: escritor.h escritor.c
	$(CC) $(CFLAGS) $(PTHREAD) -o escritor escritor.c

leitor: leitor.h leitor.c buffer.c buffer.h
	$(CC) $(CFLAGS) $(PTHREAD) -o leitor leitor.c buffer.c

monitor: monitor.c monitor.h
	$(CC) $(CFLAGS) $(PTHREAD) -o monitor monitor.c


clean:
	rm -f leitor escritor monitor
	@echo "Tudo limpo!"
