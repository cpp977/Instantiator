#include <algorithm>
#include <cstddef>

#include "Math/Matrix.hpp"
#include "Math/three_by_three.hpp"

namespace Math {

template <typename Scalar, std::size_t Rows, std::size_t Cols>
std::size_t Matrix<Scalar, Rows, Cols>::index(std::size_t row, std::size_t col) const
{
    return row + col * Rows;
}

template <typename Scalar, std::size_t Rows, std::size_t Cols>
const Scalar& Matrix<Scalar, Rows, Cols>::operator()(std::size_t row, std::size_t col) const
{
    return m_data[index(row, col)];
}

template <typename Scalar, std::size_t Rows, std::size_t Cols>
Scalar& Matrix<Scalar, Rows, Cols>::operator()(std::size_t row, std::size_t col)
{
    return m_data[index(row, col)];
}

template <typename Scalar, std::size_t Rows, std::size_t Cols>
void Matrix<Scalar, Rows, Cols>::setIdentity()
{
    for(auto i = 0ul; i < Rows; ++i) { m_data[index(i, i)] = 1.; }
}

template <typename Scalar, std::size_t Rows, std::size_t Cols>
void Matrix<Scalar, Rows, Cols>::
    setVeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeryLoooooooooooooooooooooooooooooooooooooooooooooooongIdentity()
{
    for(auto i = 0ul; i < Rows; ++i) { m_data[index(i, i)] = 1.; }
}

} // namespace Math

#if __has_include("Matrix.gen.cpp")
#    include "Matrix.gen.cpp"
#endif
