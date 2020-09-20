CC = g++
CFLAGS = -O2 -std=c++11 -Wall -Wextra -Wfloat-equal
UTDIR = ..\unit_tests

all: quadratic_equation_solver.o

quadratic_equation_solver.o: quadratic_equation_solver.cpp quadratic_equation_solver.h
	$(CC) -c quadratic_equation_solver.cpp $(CFLAGS) -I.

run_tests: run_tests.o quadratic_equation_solver.o $(UTDIR)\windows_unit_tests.o
	$(CC) -o run_tests run_tests.o quadratic_equation_solver.o $(UTDIR)\windows_unit_tests.o

run_tests.o: run_tests.cpp quadratic_equation_solver.h $(UTDIR)\windows_unit_tests.h
	$(CC) -c run_tests.cpp $(CFLAGS) -I$(UTDIR)

$(UTDIR)/windows_unit_tests.o: $(UTDIR)\windows_unit_tests.cpp $(UTDIR)\windows_unit_tests.h
	$(CC) -c $(UTDIR)\windows_unit_tests.cpp $(CFLAGS) -I$(UTDIR)

test: run_tests
	./run_tests

clean:
	del *.o run_tests.exe $(UTDIR)\*.o
