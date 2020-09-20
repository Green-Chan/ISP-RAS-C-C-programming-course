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
//! @param [out] first_root  Reference to the first root
//! @param [out] second_root  Reference to the second root
//!
//! @return Number of roots
//!
//! @note If the number of roots is infinite, returns @c INF_ROOTS.
//!       If the function returns 1, the root is written in @c first_root.
//!       If the function returns 2, @c first_root is calculated by the formula \f$ \frac{-b + \sqrt{D}}{2a} \f$,
//!       @c second_root is calculated by formula \f$ \frac{-b + \sqrt{D}}{2a} \f$, where D is the discriminant.
//!
///-------------------------------------------------------------------------------------

int solve_quadratic_equation(double a, double b, double c, double &first_root, double &second_root);


///-------------------------------------------------------------------------------------
//! Solves the linear equation \f$ a x + b = 0 \f$
//!
//! @param [in]  a   The linear coefficient (coefficient a)
//! @param [in]  b   The constant           (coefficient b)
//! @param [out] root Reference to the root
//!
//! @return Number of roots
//!
//! @note If the number of roots is infinite, returns @c INF_ROOTS.
//!
///-------------------------------------------------------------------------------------

int solve_linear_equation(double a, double b, double &root);

///-------------------------------------------------------------------------------------
//! Calculates the discriminant of the quadratic equation \f$ a x^2 + b x + c = 0 \f$
//!
//! @param [in]  a   The quadratic coefficient (coefficient a)
//! @param [in]  b   The linear coefficient    (coefficient b)
//! @param [in]  c   The constant              (coefficient c)
//!
//! @return discriminant
//!
///-------------------------------------------------------------------------------------

double calculate_discriminant(double a, double b, double c);


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
