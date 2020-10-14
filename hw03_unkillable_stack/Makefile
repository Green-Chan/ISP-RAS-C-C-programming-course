CC = gcc

DEBUG = 0
ifeq ($(DEBUG),0)
	CFLAGS = -std=c99 -O2 -Wall -Wextra -DDEBUG=0
else
	CFLAGS = -std=c99 -g -O0 -Wall -Wextra -DDEBUG=$(DEBUG)
endif

run_tests: run_tests.c stack.h
	$(CC) -o run_tests run_tests.c $(CFLAGS)

test: run_tests
	./run_tests

clean:
	del *.o *.exe
