# Instantiator
clang based tool to automatically insert all needed explicit instantiations in implementation files for `c++` projects.

# Motivation
The source code organization for c++ projects consists of header files containing the declarations and source files containing the implementation.
For example, a header file for a `class Point` could look as follows:
~~~{.cpp}
//Point.hpp
#ifndef POINT_HPP_
#define POINT_HPP_

class Point {
public:
    Point() {}
    
    Point(const double x_in, const double y_in);
    
    double norm() const;
    void shift(const Point other);
    
private:
    double x=0., y=0.;
};

#endif
~~~
The corresponding source file would then be:
~~~{.cpp}
//Point.cpp
#include "Point.hpp"

#include <cmath>

Point::Point(const double x_in, const double y_in) {
    x=x_in;
    y=y_in;
}
    
double Point::norm() const {
    return std::sqrt(x*x+y*y);
}

void Point::shift(const Point other) {
    x += other.x;
    y += other.y;
}
~~~

This decomposition has several advantages:
  * The header file defines a clear interface which can be used to see the content of the class. The concrete implementation is hidden.
  * The source file can be compiled **separately** into an object file.
  * The header file needs less `include` directives.
  * When the implementation needs to be changed, only the source file needs to recompiled and **not** other files which uses the class by including the header.

However, the `Point` class above is not generic as it has the fixed type `double` for the coordinates.
This can be changed easily because generic programming is well supported by c++ using `templates`.
The generic `Point` class header is (without the usage of `concepts`):
~~~{.cpp}
//Point.hpp
#ifndef POINT_HPP_
#define POINT_HPP_

template<typename Scalar>
class Point {
public:
    Point() {}
    
    Point(const Scalar x_in, const Scalar y_in);
    
    Scalar norm() const;
    void shift(const Point other);
    
private:
    Scalar x=0., y=0.;
};

#endif
~~~
And the corresponding source file would be:
~~~{.cpp}
//Point.cpp
#include "Point.hpp"

#include <cmath>

template<typename Scalar>
Point<Scalar>::Point(const Scalar x_in, const Scalar y_in) {
    x=x_in;
    y=y_in;
}

template<typename Scalar>
Scalar Point<Scalar>::norm() const {
    return std::sqrt(x*x+y*y);
}

template<typename Scalar>
void Point<Scalar>::shift(const Point other) {
    x += other.x;
    y += other.y;
}
~~~
Now, it becomes a problem that the declaration and the definition are separated.
When the source file `Point.cpp` is compiled to an object file, the compiler does not know a type for `Scalar`. 
It can not generate any object code. This is written in the c++ standard:
> A class template by itself is not a type, or an object, or any other entity. No code is generated from a source file that contains only template definitions. In order for any code to appear, a template must be instantiated: the template arguments must be provided so that the compiler can generate an actual class (or function, from a function template).
To resolve this issue, one can put the definition into the header file. This shifts the object/binary code generation to the compilation step where the header is included.
Assuming that a concrete type for `Scalar` is given in this file, all the class methods of `Point` can be compiled.
But with this solution, one loses the advantages from above.
Another possibility is to insert one or several explicit instantiations into `Point.cpp` to give the compiler one or several concrete types for `Scalar`.
Adding for example 
~~~{.cpp}
    template class Point<double>;
~~~
at the end of `Point.cpp` would cause object code for all members of `Point` for the type `double`.
With this solution one can keep separate translation units but it has other drawbacks:
  * The `Point` class can not be used with any type but only with those where there is an explicit instantiation.
  * The explicit class instantiation causes the compilation of **all** members. However if the class is implicitly instantiated, only code for the **used** members is generated. Implicit instantiation would occur if you put the definiton into the header and take the concrete types from the usage of the class.
  * All the explicit instantiations needs to be inserted manually which causes extra maintenance work.
  
Note, that the second point could be circumvented by not using explicit class instantiations but only explicitly instantiate certain members.
For example
~~~{.cpp}
    template void Point<double>::shift(const Point<double> other);
~~~
Buth this approach is not maintainable for large class templates.

This tool aims to provide a third solution, namely to inject the explicit member instantiations (or free function instantiations) automatically by using the information from a final main file.
This means that the `Point` class is separated in header and source file but the source file is **not** compiled to library. 
A user of the `Point` class will include a cmake target to build the library for `Point` with the dependency `Point.cpp`.
Then the main target can become the `Point` library as a dependency. 
With this setup, the tool can insert all needed explicit instantiations into `Point.cpp` by using the information from the main target.
This is achieved by using the powerful `clang` library libtooling for analyzing clangs abstract syntax tree (AST).

# The clang AST
See the [clang ast introduction](https://clang.llvm.org/docs/IntroductionToTheClangAST.html).

# Two step procedure
The tool is scanning the AST twice. 
1. It is looking for needed instantations and build a todo List of Injection entries.
2. It is searching where to place the explicit instantiations.
