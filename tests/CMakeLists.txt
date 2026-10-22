# The solver's suite: every puzzle in puzzles/ solved and checked step by step, each technique held to the solutions,
# the game, the files.
add_executable(sudoku_solver_test
    solver/game_test.cpp
    solver/puzzle_file_test.cpp
    solver/puzzle_test.cpp
    solver/technique_test.cpp)
target_link_libraries(sudoku_solver_test PRIVATE sudoku_solver sudoku_warnings GTest::gtest_main)
target_compile_definitions(sudoku_solver_test PRIVATE SUDOKU_PUZZLES_DIR="${PROJECT_SOURCE_DIR}/puzzles")
# The test list is read when the tests run, not when they are built: a build never waits on the binary starting. Each
# parameterised test is named by its own name ("xwing1_sdk", "XYChain"), not by its parameter's printed value.
gtest_discover_tests(sudoku_solver_test DISCOVERY_MODE PRE_TEST NO_PRETTY_VALUES)
