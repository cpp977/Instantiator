#!/usr/bin/env bats

setup_file() {
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
    [ "$status" -eq 0 ]
}

@test "Run Instantiator to generate template instantantions." {
    run Instantiator -p "${SCRIPT_DIR}/build/compile_commands.json" "${SCRIPT_DIR}/Calculator/src/Calculator.cpp"
    [ "$status" -eq 0 ]
}

@test "Build the executable." {
    run cmake --build "${SCRIPT_DIR}/build"
    [ "$status" -eq 0 ]
}

@test "Run Instantiator to remove template instantantions." {
    run Instantiator -p "${SCRIPT_DIR}/build/compile_commands.json" --clean "${SCRIPT_DIR}/Calculator/src/Calculator.cpp"
    [ "$status" -eq 0 ]
}

@test "Reformat the codebase with clang-format." {
    run bash -c "find \"${SCRIPT_DIR}\" -iname \"*.hpp\" -o -iname \"*.cpp\" -print0 | xargs -0 clang-format -i"
    [ "$status" -eq 0 ]
}

@test "Run git diff to check that the original version of code is restored." {
    echo git --no-pager -C "${SCRIPT_DIR}/.." diff "${SCRIPT_DIR}/Math" >&3
    run git --no-pager -C "${SCRIPT_DIR}/.." diff --quiet "${SCRIPT_DIR}/Math"
    [ "$status" -eq 0 ]
}
