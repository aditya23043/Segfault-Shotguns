CC = gcc
CFLAGS = -g -lraylib

all: main

main: main.o
	${CC} obj/main.o -o bin/main ${CFLAGS}

main.o: src/main.c
	${CC} -c src/main.c -o obj/main.o ${CFLAGS}

run: bin/main
	@ ./bin/main
