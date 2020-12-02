CC = gcc

DEBUG = 0
ifeq ($(DEBUG),0)
	CFLAGS = -std=c99 -O2 -Wall -Wextra
else
	CFLAGS = -std=c99 -g -O0 -Wall -Wextra
endif

run_tests: expression_tree.o run_tests.o
	$(CC) -o run_tests  run_tests.o expression_tree.o $(CFLAGS)

run_tests.o: run_tests.c expression_tree.h
	$(CC) -c run_tests.c $(CFLAGS)

expression_tree.o: expression_tree.c expression_tree.h
	$(CC) -c expression_tree.c $(CFLAGS)

test: run_tests
	run_tests.exe < test.in > test.out
	ECHO n | COMP test.out test_ok.out 2> nul

clean:
	del *.o *.exe
