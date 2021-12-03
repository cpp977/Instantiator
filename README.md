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
  - It builds the abstract syntax tree (AST) of all files listed in the `compile_commands.json`. This is often too much and slows down the process.
  - `Template template` parameters are not implemented.

# Known issues
  - The cleanup of explicit instantiations can mess up the files if the explicit instantiations spread over more than one line. 
    Be sure to backup everything before running the cleanup.

# Roadmap for version 1.0.0
  - Noninvasive mode: instantiations are not inserted directly to the source file but in extra files which can then be included in the source files.
  - ASTs are only build for the dependency tree and not for all files in the compile_commands.json
  - Example c++ project which serves also as a test case.
  
# How does it work?
See the [documentation](https://cpp977.github.io/Instantiator/).

# Contributions
Contributions are very welcome!
The following is only a part of things which needs to be done:
  - Adapt llvm error handling (https://llvm.org/docs/ExceptionHandling.html).
  - Clean up the `cmake` file.
  - Add an example `c++` project which can also be used for tests.

# Acknowledgments
The tool is heavily based on clang and its [libtooling](https://clang.llvm.org/docs/LibTooling.html) library which is of course acknowledged.
Beside that, the tool uses some small c++ utility libraries which are also acknowledged:
  1. [termcolor](https://github.com/ikalnytskyi/termcolor) — for colored logging output
  2. [doxygen layout](https://github.com/jothepro/doxygen-awesome-css) — for pretty online documentation
