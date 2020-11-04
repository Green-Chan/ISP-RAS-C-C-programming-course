CC = gcc

DEBUG = 0
ifeq ($(DEBUG),0)
	CFLAGS = -std=c99 -O2 -Wall -Wextra
else
	CFLAGS = -std=c99 -g -O0 -Wall -Wextra
endif

STDIR = ..\hw03_unkillable_stack

run_tests: run_tests.o assembler.o disassembler.o processor.o
	$(CC) -o run_tests  run_tests.o assembler.o disassembler.o processor.o $(CFLAGS)

run_tests.o: run_tests.c assembler.h processor.h
	$(CC) -c run_tests.c

assembler.o: assembler.c assembler.h processor.h $(STDIR)\stack.h
	$(CC) -c assembler.c -I$(STDIR)

disassembler.o: disassembler.c assembler.h processor.h
	$(CC) -c disassembler.c

processor.o: processor.c assembler.h processor.h $(STDIR)\stack.h
	$(CC) -c processor.c -I$(STDIR)

test: run_tests
	run_tests.exe < test.in

clean:
	del *.o *.exe
