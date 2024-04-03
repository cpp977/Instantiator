#ifndef MATH_MATRIX_H_
#define MATH_MATRIX_H_

#include <array>
#include <cstddef>

#include "MatrixBase.hpp"

namespace Math {

template <typename Scalar_, std::size_t Rows_, std::size_t Cols_>
struct MatrixTraits<Matrix<Scalar_, Rows_, Cols_>>
{
    using Scalar = Scalar_;
    static constexpr std::size_t Rows = Rows_;
    static constexpr std::size_t Cols = Cols_;
};

template <typename Scalar_, std::size_t Rows_, std::size_t Cols_>
class Matrix : public MatrixBase<Matrix<Scalar_, Rows_, Cols_>>
{
public:
    using Scalar = Scalar_;
    static constexpr std::size_t Rows = Rows_;
    static constexpr std::size_t Cols = Cols_;

    Matrix() = default;

    Matrix(const Matrix& other) = default;

    Matrix(Matrix&& other) = default;

    constexpr auto rows() const { return Rows; }
    constexpr auto cols() const { return Cols; }

    const Scalar& operator()(std::size_t row, std::size_t col) const;
    Scalar& operator()(std::size_t row, std::size_t col);

    void setIdentity();

    void
    setVeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeryLoooooooooooooooooooooooooooooooooooooooooooooooongIdentity();

private:
    std::size_t index(std::size_t row, std::size_t col) const;
    std::array<Scalar, Rows * Cols> m_data{};
};

} // namespace Math

#endif
