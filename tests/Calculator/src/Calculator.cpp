#include <iostream>

#include "Math/CoeffUnaryOp.hpp"
#include "Math/three_by_three.hpp"
#include "Math/Matrix.hpp"

int main()
{
    Math::Matrix<double, 4, 4> m;
    m.setIdentity();
    double t = m.trace();
    std::cout << "trace=" << t << std::endl;
    Math::Matrix<double, 10, 10> n;
    n.setVeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeryLoooooooooooooooooooooooooooooooooooooooooooooooongIdentity();
    double q = n.trace();
    std::cout << "trace=" << q << std::endl;
    const auto x = n.square();
    std::cout << "x(0,0)=" << x(0, 0) << std::endl;
}
