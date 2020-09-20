#include "quadratic_equation_solver.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>

/* See description in quadratic_equation_solver.h */

int solve_quadratic_equation(double a, double b, double c, double &first_root, double &second_root)
{
    assert(std::isfinite(a));
    assert(std::isfinite(b));
    assert(std::isfinite(c));
    first_root = NAN;
    second_root = NAN;

    if (is_zero(a)) {
        return solve_linear_equation(b, c, first_root);
    } else { // a != 0
        double discriminant = calculate_discriminant(a, b, c);
        if (is_zero(discriminant)) {
            first_root = -b / (2 * a);
            if (!std::isfinite(first_root)) {
                throw std::runtime_error("Got not finite value, calculating the root of the quadratic equation "
                                         + std::to_string(a) + " * x^2 + " + std::to_string(b) + " * x + " +
                                         std::to_string(c) + "in file: " + __FILE__);
            }
            return 1;
        } else if (discriminant < 0) {
            return 0;
        } else { // discriminant > 0
            assert(&first_root != &second_root);
            first_root = (-b + sqrt(discriminant)) / (2 * a);
            second_root = (-b - sqrt(discriminant)) / (2 * a);
            if (!std::isfinite(first_root) || !std::isfinite(second_root)) {
                throw std::runtime_error("Got not finite value, calculating the root of the quadratic equation "
                                         + std::to_string(a) + " * x^2 + " + std::to_string(b) + " * x + " +
                                         std::to_string(c) + "in file: " + __FILE__);
            }
            return 2;
        }
    }
}


/* See description in quadratic_equation_solver.h */

int solve_linear_equation(double a, double b, double &root)
{
    assert(std::isfinite(a));
    assert(std::isfinite(b));
    root = NAN;

    if (is_zero(a)) {
        if (is_zero(b)) {
            return INF_ROOTS;
        } else {
            return 0;
        }
    } else { // b != 0
        root = - b / a;
        if (!std::isfinite(root)) {
            throw std::runtime_error("Got not finite value, calculating the root of the linear equation "
                                     + std::to_string(a) + " * x + " + std::to_string(b) + "in file: " + __FILE__);
        }
        return 1;
    }
}


/* See description in quadratic_equation_solver.h */

double calculate_discriminant(double a, double b, double c)
{
    assert(std::isfinite(a));
    assert(std::isfinite(b));
    assert(std::isfinite(c));

    double discriminant = b * b - 4 * a * c;
    if (!std::isfinite(discriminant)) {
        throw std::runtime_error("Got not finite value, calculating the discriminant of the quadratic equation "
                                 + std::to_string(a) + " * x^2 + " + std::to_string(b) + " * x + " +
                                 std::to_string(c) + "in file: " + __FILE__);
    }
    return discriminant;
}


/* See description in quadratic_equation_solver.h */

bool is_zero(double num)
{
    assert(std::isfinite(num));
    return std::fabs(num) < EPS;
}
