#Makefile - TP2 (Clinica)

#Variables:
EXEC=tp
CC=gcc
CFLAGS= -std=c99 -g -Wall -Wconversion -Wtype-limits -pedantic -Werror
OBJECTS=abb.o clinica.o cola.o csv.o hash.o heap.o lista.o pila.o
VALGRIND= valgrind --leak-check=full --track-origins=yes

all: $(EXEC)

abb.o: abb.c abb.h
	$(CC) $(CFLAGS) -c abb.c

clinica.o: clinica.c clinica.h
	$(CC) $(CFLAGS) -c clinica.c

cola.o: cola.c cola.h
	$(CC) $(CFLAGS) -c cola.c

csv.o: csv.c csv.h
	$(CC) $(CFLAGS) -c csv.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

heap.o: heap.c heap.h
	$(CC) $(CFLAGS) -c heap.c

lista.o: lista.c lista.h
	$(CC) $(CFLAGS) -c lista.c

pila.o: pila.c pila.h
	$(CC) $(CFLAGS) -c pila.c

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXEC)

valgrind: $(EXEC)
	$(VALGRIND) ./$(EXEC)

clean: 
	rm -f *.o *~
