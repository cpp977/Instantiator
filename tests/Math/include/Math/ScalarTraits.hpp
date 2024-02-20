#ifndef MATH_SCALAR_TRAITS_H_
#define MATH_SCALAR_TRAITS_H_

#include <complex>

namespace Math {

template <typename Scalar>
struct ScalarTraits
{};

template <>
struct ScalarTraits<double>
{
    typedef double Real;
    typedef std::complex<double> Comp;
    static inline double epsilon() { return 1.e-12; }
    static constexpr bool IS_COMPLEX() { return false; }
};

template <typename RealScalar_>
struct ScalarTraits<std::complex<RealScalar_>>
{
    typedef RealScalar_ Real;
    typedef std::complex<RealScalar_> Comp;
    static inline Real epsilon() { return static_cast<Real>(1.e-12); }
    static constexpr bool IS_COMPLEX() { return true; }
};

} // namespace Math
#endif
