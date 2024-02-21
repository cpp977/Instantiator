#include <cmath>
#include <complex>

#include "Math/CoeffUnaryOp.hpp"
#include "Math/Matrix.hpp"
#include "Math/MatrixBase.hpp"
#include "Math/ScalarTraits.hpp"

namespace Math {

template <typename Derived>
const Derived& MatrixBase<Derived>::derived() const
{
    return *static_cast<const Derived*>(this);
}

template <typename Derived>
Derived& MatrixBase<Derived>::derived()
{
    return *static_cast<Derived*>(this);
}

template <typename Derived>
typename MatrixTraits<Derived>::Scalar MatrixBase<Derived>::trace() const
{
    static_assert(MatrixTraits<Derived>::Rows == MatrixTraits<Derived>::Rows, "Need a square matrix for trace.");
    auto& d = derived();
    Scalar out = 0.;
    for(auto i = 0ul; i < d.rows(); ++i) { out += d(i, i); }
    return out;
}

template <typename Derived>
template <typename ReturnScalar>
const CoeffUnaryOp<Derived, ReturnScalar> MatrixBase<Derived>::unaryExpr(const std::function<ReturnScalar(Scalar)>& coeff_func) const
{
    return CoeffUnaryOp<Derived, ReturnScalar>(derived(), coeff_func);
}

template <typename Derived>
const CoeffUnaryOp<Derived, typename MatrixTraits<Derived>::Scalar> MatrixBase<Derived>::sqrt() const
{
    return unaryExpr<Scalar>([](Scalar s) { return std::sqrt(s); });
}

template <typename Derived>
const CoeffUnaryOp<Derived, typename MatrixTraits<Derived>::Scalar> MatrixBase<Derived>::square() const
{
    return unaryExpr<Scalar>([](Scalar s) { return s * s; });
}

template <typename Derived>
const CoeffUnaryOp<Derived, typename MatrixTraits<Derived>::Scalar> MatrixBase<Derived>::inv() const
{
    return unaryExpr<Scalar>([](Scalar s) { return 1. / s; });
}

template <typename Derived>
const CoeffUnaryOp<Derived, typename ScalarTraits<typename MatrixTraits<Derived>::Scalar>::Real> MatrixBase<Derived>::real() const
{
    return unaryExpr([](Scalar s) { return std::real(s); });
}

template <typename Derived>
const CoeffUnaryOp<Derived, typename ScalarTraits<typename MatrixTraits<Derived>::Scalar>::Real> MatrixBase<Derived>::imag() const
{
    return unaryExpr([](Scalar s) { return std::imag(s); });
}

template <typename Derived>
template <typename OtherScalar>
const CoeffUnaryOp<Derived, OtherScalar> MatrixBase<Derived>::cast() const
{
    return unaryExpr([](Scalar s) { return static_cast<OtherScalar>(s); });
}

} // namespace Math
