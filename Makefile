#Makefile - TP2 (Clinica)

#Variables:
EXEC=tp
CC=gcc
CFLAGS= -std=c99 -g -Wall -Wconversion -Wtype-limits -pedantic -Werror
OBJECTS=abb.o clinica.o cola.o csv.o hash.o heap.o lista.o pila.o
VALGRIND= valgrind --leak-check=full --track-origins=yes

all: $(EXEC)

abb: abb.c abb.h
	$(CC) $(CFLAGS) -c abb.c

clinica: clinica.c clinica.h
	$(CC) $(CFLAGS) -c clinica.c

cola: cola.c cola.h
	$(CC) $(CFLAGS) -c cola.c

csv: csv.c csv.h
	$(CC) $(CFLAGS) -c csv.c

hash: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

heap: heap.c heap.h
	$(CC) $(CFLAGS) -c heap.c

lista: lista.c lista.h
	$(CC) $(CFLAGS) -c lista.c

pila: pila.c pila.h
	$(CC) $(CFLAGS) -c pila.c

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXEC)

valgrind: $(EXEC)
	$(VALGRIND) ./$(EXEC)

clean: 
	rm -f *.o *~
