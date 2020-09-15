all: quadratic_equation_solver.o

quadratic_equation_solver.o: quadratic_equation_solver.cpp quadratic_equation_solver.h
	g++ -c quadratic_equation_solver.cpp -O2 -std=c++11

run_tests: run_tests.o quadratic_equation_solver.o windows_unit_tests.o
	g++ -o run_tests run_tests.o quadratic_equation_solver.o windows_unit_tests.o

run_tests.o: run_tests.cpp quadratic_equation_solver.h ../unit_tests/windows_unit_tests.h
	g++ -c run_tests.cpp -O2 -std=c++11 -I"../unit_tests"

windows_unit_tests.o: ../unit_tests/windows_unit_tests.cpp ../unit_tests/windows_unit_tests.h
	g++ -c ../unit_tests/windows_unit_tests.cpp -O2 -std=c++11 -I"../unit_tests"

test: run_tests
	./run_tests

clean:
	del *.o run_tests.exe
