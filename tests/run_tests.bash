#!/usr/bin/env bats

setup_file() {
    load '/usr/lib/bats/bats-support/load.bash'
    load '/usr/lib/bats/bats-assert/load.bash'

    # get the containing directory of this file
    SCRIPT_DIR="$( cd "$( dirname "$BATS_TEST_FILENAME" )" >/dev/null 2>&1 && pwd )"
    export SCRIPT_DIR
    # Create build directory
    mkdir "$SCRIPT_DIR/build"
    export CXX=clang++
    export CXXFLAGS="-isystem /usr/lib/clang/16/include/"
}

teardown_file() {
    rm -rf "${SCRIPT_DIR}/build"
}

@test "Configure the test project with CMake." {
    run cmake -S "${SCRIPT_DIR}/Calculator/" -B "${SCRIPT_DIR}/build"
}

@test "Run Instantiator to generate template instantantions." {
    run Instantiator -p "${SCRIPT_DIR}/build/compile_commands.json" "${SCRIPT_DIR}/Calculator/src/Calculator.cpp"
}

@test "Build the executable." {
    cmake --build "${SCRIPT_DIR}/build"
}

@test "Run Instantiator to remove template instantantions." {
    Instantiator -p "${SCRIPT_DIR}/build/compile_commands.json" --clean "${SCRIPT_DIR}/Calculator/src/Calculator.cpp"
}

@test "Reformat the codebase with clang-format." {
    find "${SCRIPT_DIR}" -iname "*.hpp" -print0 -o -iname "*.cpp" | xargs clang-format -i
}

@test "Run git diff to check that the original version of code is restored." {
    git --no-pager -C "${SCRIPT_DIR}/.." diff --quiet "${SCRIPT_DIR}/Math"
}
