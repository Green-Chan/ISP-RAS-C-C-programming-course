CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

run_tests: run_tests.c stack.h
	$(CC) -o run_tests run_tests.c $(CFLAGS)

test: run_tests
	./run_tests

clean:
	del *.o *.exe
