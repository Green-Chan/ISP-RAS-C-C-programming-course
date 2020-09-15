#include "quadratic_equation_solver.h"
#include "windows_unit_tests.h"

#include <iostream>
#include <cmath>

#define $test_qes(code, expected_nroots, expected_x1, expected_x2) \
{                                                                  \
    $unit_test(code, expected_nroots);                             \
    if (expected_nroots > 0) {                                     \
        $f_unit_test(x1, expected_x1, 1e-5);                       \
    }                                                              \
    if (expected_nroots > 1) {                                     \
        $f_unit_test(x2, expected_x2, 1e-5);                       \
    }                                                              \
    std::cout << std::endl;                                        \
}

int main() {
    double x1, x2;

    $test_qes(solve_quadratic_equation(1, -5,  6, x1, x2), 2,  3, 2);
    $test_qes(solve_quadratic_equation(1,  2,  1, x1, x2), 1, -1, 0);
    $test_qes(solve_quadratic_equation(1,  1,  1, x1, x2), 0,  0, 0);
    $test_qes(solve_quadratic_equation(0,  2, -2, x1, x2), 1,  1, 0);
    $test_qes(solve_quadratic_equation(0,  0, -2, x1, x2), 0,  0, 0);
    $test_qes(solve_quadratic_equation(0,  0,  0, x1, x2), INF_ROOTS,  0, 0);

    $test_qes(solve_quadratic_equation(-2,  3, -0.2, x1, x2), 2,  (-3 + sqrt(9 - 1.6)) / (-4), (-3 - sqrt(9 - 1.6)) / (-4));
    $test_qes(solve_quadratic_equation( 2,  3, -0.2, x1, x2), 2,  (-3 + sqrt(9 + 1.6)) /   4 , (-3 - sqrt(9 + 1.6)) /   4 );
    $test_qes(solve_quadratic_equation(-2, -3, -0.2, x1, x2), 2,  ( 3 + sqrt(9 - 1.6)) / (-4), ( 3 - sqrt(9 - 1.6)) / (-4));
    $test_qes(solve_quadratic_equation(0.1, 3,    2, x1, x2), 2,  (-3 + sqrt(9 - 0.8)) / 0.2 , (-3 - sqrt(9 - 0.8)) / 0.2 );

    $test_qes(solve_quadratic_equation(-0.1, sqrt(1.2), -3, x1, x2), 1,  sqrt(1.2) / 0.2, 0);
    $test_qes(solve_quadratic_equation( 0.1, sqrt(5.2), 13, x1, x2), 1, -sqrt(5.2) / 0.2, 0);

    $test_qes(solve_quadratic_equation(0.2,  0.3,  20, x1, x2), 0,  0, 0);
    $test_qes(solve_quadratic_equation(-20,   3, -0.2, x1, x2), 0,  0, 0);

    $test_qes(solve_quadratic_equation(0, 0.5, 7, x1, x2), 1,  -14, 0);
    $test_qes(solve_quadratic_equation(sqrt(1.2) * sqrt(1.2) - 0.1 * 3 * 4, -7, 0.5, x1, x2), 1, 0.5 / 7, 0);

    $test_qes(solve_quadratic_equation(sqrt(1.2) * sqrt(1.2) - 0.1 * 3 * 4, 0, -25, x1, x2), 0,  0, 0);
    $test_qes(solve_quadratic_equation(0, sqrt(1.2) * sqrt(1.2) - 0.1 * 3 * 4, 0.2, x1, x2), 0,  0, 0);

    $test_qes(solve_quadratic_equation(0, 0, sqrt(1.2) * sqrt(1.2) - 0.1 * 3 * 4, x1, x2), INF_ROOTS, 0, 0);

    $unit_test_sigabrt(solve_quadratic_equation(0.1, 3, 2, x1, x1));

    $unit_test_sigabrt(solve_quadratic_equation(NAN, 3, 2, x1, x2));
    $unit_test_sigabrt(solve_quadratic_equation(0.1, NAN, 2, x1, x2));
    $unit_test_sigabrt(solve_quadratic_equation(0.1, 3, NAN, x1, x2));

    return 0;
}
