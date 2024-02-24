#include <algorithm>
#include <cstddef>

#include "Math/CoeffUnaryOp.hpp"
#include "Math/Matrix.hpp"

namespace Math {

template <typename XprType, typename ReturnScalar>
ReturnScalar CoeffUnaryOp<XprType, ReturnScalar>::operator()(std::size_t row, std::size_t col) const
{
    return m_func(m_refexpr(row, col));
}

} // namespace Math
