# The solver: the board, the techniques, the solver that tries them, the search that checks a puzzle, the puzzle files and
# the game a person works on - written against nothing but the standard library.
add_library(sudoku_solver STATIC
    solver/brute_force.cpp
    solver/digits.cpp
    solver/game.cpp
    solver/geometry.cpp
    solver/grid.cpp
    solver/puzzle_bank.cpp
    solver/puzzle_file.cpp
    solver/solver.cpp
    solver/step.cpp
    solver/techniques/chains.cpp
    solver/techniques/fish.cpp
    solver/techniques/forcing.cpp
    solver/techniques/intersections.cpp
    solver/techniques/marks.cpp
    solver/techniques/singles.cpp
    solver/techniques/subsets.cpp
    solver/techniques/wings.cpp)
target_include_directories(sudoku_solver PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(sudoku_solver PRIVATE sudoku_warnings)

if(SUDOKU_GUI)
    add_subdirectory(app)
endif()
