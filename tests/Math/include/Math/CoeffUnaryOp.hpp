#ifndef MATH_COEFF_UNARY_OP_H_
#define MATH_COEFF_UNARY_OP_H_

#include <array>
#include <cstddef>
#include <functional>

#include "MatrixBase.hpp"

namespace Math {

template <typename XprType, typename ReturnScalar>
struct MatrixTraits<CoeffUnaryOp<XprType, ReturnScalar>>
{
    using Scalar = ReturnScalar;
    static constexpr std::size_t Rows = XprType::Rows;
    static constexpr std::size_t Cols = XprType::Cols;
};

template <typename XprType_, typename ReturnScalar>
class CoeffUnaryOp : public MatrixBase<CoeffUnaryOp<XprType_, ReturnScalar>>
{
public:
    using XprType = XprType_;
    using Scalar = ReturnScalar;
    static constexpr std::size_t Rows = XprType::Rows_;
    static constexpr std::size_t Cols = XprType::Cols_;

    CoeffUnaryOp(const XprType& xpr, const std::function<ReturnScalar(typename XprType::Scalar)>& coeff_func)
        : m_refexpr(xpr)
        , m_func(coeff_func)
    {}

    constexpr auto rows() const { return Rows; }
    constexpr auto cols() const { return Cols; }

    Scalar operator()(std::size_t row, std::size_t col) const;

protected:
    const XprType& m_refexpr;
    const std::function<ReturnScalar(typename XprType::Scalar)> m_func;
};

} // namespace Math

#endif
