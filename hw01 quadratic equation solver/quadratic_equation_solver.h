#ifndef __QUADRATIC_EQUATION_SOLVER_HEADER
#define __QUADRATIC_EQUATION_SOLVER_HEADER

constexpr int INF_ROOTS = -1;
constexpr double EPS = 1e-5;

///-------------------------------------------------------------------------------------
//! Solves the quadratic equation \f$ a x^2 + b x + c = 0 \f$
//!
//! @param [in]  a   The quadratic coefficient (coefficient a)
//! @param [in]  b   The linear coefficient    (coefficient b)
//! @param [in]  c   The constant              (coefficient c)
//! @param [out] x1  Reference to the 1st root
//! @param [out] x2  Reference to the 2nd root
//!
//! @return Number of roots
//!
//! @note If the number of roots is infinite, returns @c INF_ROOTS.
//!       If the function returns 1, the root is written in @c x1.
//!
///-------------------------------------------------------------------------------------

int solve_quadratic_equation(double a, double b, double c, double &x1, double &x2);


///-------------------------------------------------------------------------------------
//! Comparing double to zero
//!
//! @param [in] num The double number
//!
//! @return True if @c num is close to zero, false otherwise
//!
//! @note @c num is considered close to zero, if its absolute value is less than @c EPS.
//!
///-------------------------------------------------------------------------------------

bool is_zero(double num);

#endif
