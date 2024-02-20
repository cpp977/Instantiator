#ifndef MATH_MATRIX_BASE_H_
#define MATH_MATRIX_BASE_H_

#include <cstddef>
#include <functional>

#include "Math/ScalarTraits.hpp"

namespace Math {

template <typename>
struct MatrixTraits;

template <typename, std::size_t, std::size_t>
class Matrix;

template <typename, typename>
class CoeffUnaryOp;

template <typename Derived>
class MatrixBase
{
    using Scalar = typename MatrixTraits<Derived>::Scalar;
    static constexpr std::size_t Rows = MatrixTraits<Derived>::Rows;
    static constexpr std::size_t Cols = MatrixTraits<Derived>::Cols;

public:
    const Derived& derived() const;
    Derived& derived();

    template <typename ReturnScalar>
    const CoeffUnaryOp<Derived, ReturnScalar> unaryExpr(const std::function<ReturnScalar(Scalar)>& coeff_func) const;

    const CoeffUnaryOp<Derived, Scalar> sqrt() const;
    const CoeffUnaryOp<Derived, Scalar> inv() const;
    const CoeffUnaryOp<Derived, Scalar> square() const;
    const CoeffUnaryOp<Derived, typename ScalarTraits<Scalar>::Real> abs() const;
    template <typename OtherScalar>
    const CoeffUnaryOp<Derived, OtherScalar> cast() const;
    const CoeffUnaryOp<Derived, typename ScalarTraits<Scalar>::Real> real() const;
    const CoeffUnaryOp<Derived, typename ScalarTraits<Scalar>::Real> imag() const;

    Scalar trace() const;
    Scalar norm() const;
    Scalar sum() const;
};

} // namespace Math

#endif
