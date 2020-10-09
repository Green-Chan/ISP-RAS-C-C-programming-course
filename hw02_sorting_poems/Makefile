CC = g++

DEBUG = 0
ifeq ($(DEBUG),0)
	CFLAGS = -O2 -std=c++17 -Wall -Wextra
else
	CFLAGS = -g -O0 -std=c++17 -Wall -Wextra
endif

UTDIR = ..\unit_tests

all: run_tests run_sorting

run_tests: run_tests.o $(UTDIR)\windows_unit_tests.o text_sorting.o
	$(CC) -o run_tests run_tests.o $(UTDIR)\windows_unit_tests.o text_sorting.o

run_tests.o: run_tests.cpp qsort.h $(UTDIR)\windows_unit_tests.h text_sorting.h
	$(CC) -c run_tests.cpp $(CFLAGS) -I$(UTDIR)

$(UTDIR)/windows_unit_tests.o: $(UTDIR)\windows_unit_tests.cpp $(UTDIR)\windows_unit_tests.h
	$(CC) -c $(UTDIR)\windows_unit_tests.cpp $(CFLAGS) -I$(UTDIR)

text_sorting.o: text_sorting.h text_sorting.cpp qsort.h
	$(CC) -c text_sorting.cpp $(CFLAGS)

test: run_tests
	./run_tests

clean:
	del *.o *.exe $(UTDIR)\*.o

run: run_sorting
	./run_sorting

run_sorting: run_sorting.o text_sorting.o
	$(CC) -o run_sorting run_sorting.o text_sorting.o $(CFLAGS)

run_sorting.o: run_sorting.cpp text_sorting.h
	$(CC) -c run_sorting.cpp $(CFLAGS)
