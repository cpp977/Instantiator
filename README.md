![TESTS](https://github.com/cpp977/Instantiator/actions/workflows/tests.yml/badge.svg)

# Instantiator
clang based tool to automatically insert all needed explicit instantiations in implementation files for `c++` projects.

# What does it can do?
The tool allows you to implement your function and class templates in the same manner as normal function and classes.
I.e. you can put the declarations in header files (`.hpp`) and the definitions in implementation files (`.cpp`).
If you do this without using this tool the compiled object files of the provided implementations are empty because they do not exist any template instantiations.
If you run
```bash
Instantiator -p <path/to/compile_command.json> <MAIN_CPPFILE>
```
all needed instantiations of your function and class templates will be placed automatically to the correct position.
Then you can build your project without linking errors but with all the benefits of having separate translation units:
  - Parallel compilation
  - Fast incremental rebuilds
  
The tool only adds instantiations for things you use. That ensures that the `c++` template paradigm *you only get object code for things you really use* is respected.
The compilation step involves exactly the same effort as if you had everything in the header files.

# What does it (currently) not can do?
  - It does not manage the `includes` for you. If it inserts a needed template instantiation with a type not known in that translation unit, you will get a compiler error.

# How to build?
```
git clone https://github.com/cpp977/Instantiator
mkdir build && cd build
CXX=clang++ CC=clang cmake ..
make && [sudo] make install
```
For a successful build you need a c++-17 compiler and LLVM/clang libraries.
The CI tests the most recent llvm versions (currently 17, 18, 19 and 20) and uses clang itself to build the tool.
In general, gcc should work equally well.
To control which version of LLVM is used, you can set the cmake variable `LLVM_ROOT` to the root of the llvm installation.
To control which version of clang is used, you can set the cmake variable `Clang_DIR` to the root of the directory where the file ClangConfig.cmake is located.

# Roadmap for version 2.0.0
  - Apply direct AST transformationen instead of injecting instantiations as source code and recompile afterwards.

# How does it work?
See the [documentation](https://cpp977.github.io/Instantiator/).

# Contributions
Contributions are very welcome!
The following is only a part of things which needs to be done:
  - Inspect how libtooling can do AST transformations
  - Clean up cmake build files.  - Adapt llvm error handling (https://llvm.org/docs/ExceptionHandling.html).
  - Clean up the `cmake` file.


# Acknowledgments
The tool is heavily based on clang and its [libtooling](https://clang.llvm.org/docs/LibTooling.html) library which is of course acknowledged.
Beside that, the tool uses some small c++ utility libraries which are also acknowledged:
  1. [libfmt](https://github.com/fmtlib/fmt) — for (colored) printing
  2. [spdlog](https://github.com/gabime/spdlog) — for logging
  3. [doxygen layout](https://github.com/jothepro/doxygen-awesome-css) — for pretty online documentation
