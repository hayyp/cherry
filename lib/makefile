CC=gcc
CFLAGS= -Wall -Wextra
BIN= bin

all: libco.a

libco.a: libco.o
	ar rcs $(BIN)/$@ $^

libco.o: libco/libco.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm *.o
