# Change Log
All notable changes to this project will be documented in this file.
 
The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [1.0.0] - 2024-07-XX
  - Support for noninvasive mode. Instantiations are inserted into extra source files with the naming scheme <filename>.gen.cpp.
    Those file can be excluded from version control and no generated instantiations can be committed unintentionally.
    Also the deletion of inserted instantiations become trivial and cannot mess up existing source code.
    The noninvasive mode is the recommended mode and also the default mode.
    The old mode can be used by adding the command line option `--invasive`.
  - Remove thirdparty dependency to *termcolor* and *indicators*.
  - Use *libfmt* and *spdlog* for console printing and logging.
  - Automatic sort of includes by clang-format.
  - Maintenance and clean-up.
  
## [0.3.0] - 2022-01-03
  - Support for `template template` parameters. This requires most recent version 14.0.0git of LLVM/clang.
  - Implemented disk cache for ASTs. When an AST for a source file is needed, the tool checks whether an up-to-date `.ast` file is available. 
    If so it deserialize the `.ast` file if not it (re)parses the AST from the source file and serializes it afterwards as a `.ast` file.
    If the `.ast` file is up-to-date is checked by comparing the time-stamps of all dependencies (also header files) with the time-stamp of the `.ast` file.
    This leeds to much less RAM usage for large projects but initial runs take slightly longer. However, reruns are also faster because lots of unrelated ASTs do not have to be parsed.
  - Renamed commandline option `-i` to `--ignore` but kept `-i` as an alias for `--ignore`
  - Added build and install instructions in README
  
## [0.2.1] - 2021-12-03
  - Bugfix: Some needed instantiations got deleted for the `toDoList` because they was implicitly instantiated which nevertheless led to linker errors.
    Now needed instantiations are only deleted if they are already *explicitly* instantiated.
  - Bugfix: `clean` option has messed up the files because of a known bug in `clang::Rewriter` (See this [FIXME](https://clang.llvm.org/doxygen/structclang_1_1Rewriter_1_1RewriteOptions.html#af89ac8a120822d2801ac443b35f1156b)). Now, blank lines are not deleted anymore. To clean up the blank lines, one should use `clang-format`.
  
## [0.2.0] - 2021-11-26
  - Documentation added.
  - New command line option `--clean` to remove all inserted explicit instantiations.
  - New command line option `-i` for specifiying namespaces that should be ignored. Several namespaces are specified by several `-i`.
  - Improved comparison of dependent types.
  
## [0.1.0] - 2021-10-21
Initial release with basic functionality.
