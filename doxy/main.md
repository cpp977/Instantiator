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
This means that the `Point` class is separated in header and source file but the source file is **not** compiled to a library. 
A user of the `Point` class will include a cmake target to build the library for `Point` with the dependency `Point.cpp`.
Then the main target can become the `Point` library as a dependency. 
With this setup, the tool can insert all needed explicit instantiations into `Point.cpp` by using the information from the main target.
This is achieved by using the powerful `clang` library *libtooling* for analyzing clangs abstract syntax tree (AST).

# The clang AST
To get familiar with the clang AST, see the [clang AST introduction](https://clang.llvm.org/docs/IntroductionToTheClangAST.html).
The AST is a structured version of a c++ program. It contains nodes for the different statements and declarations in the code.
Clang has two different base classes `Stmt` and `Decl` which do not have a common base.
For the purpose of this tool, the `FunctionDecl` and `CXXMethodDecl` classes are most important which both derive from `Decl`.
To filter specific nodes from the AST, clang provides [AST matchers](https://clang.llvm.org/docs/LibASTMatchersReference.html).

# General procedure
In order to inject the needed instantiations, the tool is iterating an inner two step procedure.
The iteration is necessary because an explcit instantiation in a source file can lead to new instantiations in other source files.
This is because if an explicit instantiation is inserted, new code become visible, namely the code of the definiton of the function which was instantiated.
The tool starts with the *main* translation unit, i.e. the source file in which the `main()` function is present.
This source file is added to the `working_list`.
Then the tool is processing the working list and does for each file in the working list an inner two step procedure.
The first step is the *lookup* step in which the AST of a translation unit is scanned for template instantiations for which **no** definition is present.
The result of the *lookup* step is stored as a `toDoList`.
In the second step, the *insertion* step, all other translation units are scanned if they do provide the missing definiton of an item in the `toDoList`.
If so, the corresponding explicit instantiation is inserted in the source file of this translation unit and the item is removed from the `toDoList`.
Each time, an explicit instantiation is inserted, the corresponding source file is added to the `working_list`.
The tool stops if the `working_list` is empty. 
Remaining elements in the `toDoList` might cause linking errors unless the object code for these functions is added by linking an already compiled library with the respective definiton.

## Lookup step -- find template instantiations with missing definiton ##
The *lookup* step is performed for a given source file (.cpp file). During this step, the AST of the corresponding source file is processed.
When the definiton of function templates or class template member functions is separated into a different translation unit, 
the AST may contain nodes `FunctionDecl` or `CXXMethodDecl` which are template instantiations but which have **no** definition within this AST.
The AST matcher which matches these nodes is TemplInstWithoutDef():
\snippet src/Matcher/Matcher.cpp TemplInstWithoutDef
This matcher also excludes a list of custom namespaces `excluded_names` from the search which can be passed to the tool via the commandline.
For each match, all relevant data is loaded into an `Injection` by using the factory functions `Injection::createFromFS()` and `Injection::createFromMFS()`.
## Insertion step -- inject the instantations where the definitons are present ##
The *insertion* step is performed for all other source files of the build except the source file which was scanned in the *lookup* step.
Therefore, this step involves an outerloop over the source files.
For each source file, the corresponding AST is analysed with the AST matcher FuncWithDef():
\snippet src/Matcher/Matcher.cpp FuncWithDef
This matcher takes again a list of custom namespaces `excluded_names` which will be ignored from the search.
This matcher returns any `FunctionDecl` or `CXXMethodDecl` that contains a definiton.
For each match, the tool checks whether the present function is a template and does match any of the functions from the `toDoList`.
Remember, that the `toDoList` has functions for which a definiton was *missing*, so the tool basically search if the definition is present in any other source file.
If an item of the `toDoList` does match, the tool inserts the corresponding explicit instantiation.
If on the other hand, the present function is a template specialization and matches an item of the `toDoList`, 
this item will be directly deleted, since the necessary instantiation is already present.
