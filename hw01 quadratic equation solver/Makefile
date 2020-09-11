all: quadratic_equation_solver.o

quadratic_equation_solver.o: quadratic_equation_solver.cpp quadratic_equation_solver.h
	g++ -c quadratic_equation_solver.cpp -O2 -std=c++11

run_tests: run_tests.o quadratic_equation_solver.o
	g++ -o run_tests run_tests.o quadratic_equation_solver.o

run_tests.o: run_tests.cpp quadratic_equation_solver.h
	g++ -c run_tests.cpp -O2 -std=c++11 -I"../unit tests"

test: run_tests
	./run_tests

clean:
	del *.o run_tests
