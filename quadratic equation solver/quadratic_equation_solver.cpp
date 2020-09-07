#include "quadratic_equation_solver.h"

#include <cassert>
#include <cmath>

int solve_quadratic_equation (double a, double b, double c, double &x1, double &x2)
{
    assert(std::isfinite(a));
    assert(std::isfinite(b));
    assert(std::isfinite(c));

    if (is_zero(a)) {
        if (is_zero(b)) {
            if (is_zero(c)) {
                return INF_ROOTS;
            } else {
                return 0;
            }
        } else { // b != 0
            x1 = - c / b;
            return 1;
        }
    } else { // a != 0
        double D = b * b - 4 * a * c; // Discriminant
        if (is_zero(D)) {
            x1 = -b / (2 * a);
            return 1;
        } else if (D < 0) {
            return 0;
        } else { // D > 0
            assert(&x1 != &x2);
            x1 = (-b + sqrt(D)) / (2 * a);
            x2 = (-b - sqrt(D)) / (2 * a);
            return 2;
        }
    }
}

bool is_zero (double num)
{
    return std::fabs(num) < EPS;
}
