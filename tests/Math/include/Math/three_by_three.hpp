#ifndef MATH_THREE_BY_THREE_HPP_
#define MATH_THREE_BY_THREE_HPP_

#include "Matrix.hpp"

namespace Math {

template <typename Scalar>
class Matrix<Scalar, 3, 3>
{
public:
    void setIdentity() { std::fill(m_data.begin(), m_data.end(), 1); }

private:
    std::array<Scalar, 3 * 3> m_data{};
};

} // namespace Math
#endif
