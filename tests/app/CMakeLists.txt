# The window's suite: the controller, the menus, new puzzles from the bank, the settings file and the views (painted
# into images, driven by events), with no window.
add_executable(sudoku_app_test controller_test.cpp puzzle_bank_test.cpp settings_file_test.cpp views_test.cpp)
target_link_libraries(sudoku_app_test PRIVATE sudoku_gui GTest::gtest_main sudoku_warnings)
# The repository's sample of the puzzle bank, which the suite takes for the bank.
target_compile_definitions(sudoku_app_test PRIVATE SUDOKU_PUZZLES_DIR="${PROJECT_SOURCE_DIR}/puzzles")
gtest_discover_tests(sudoku_app_test DISCOVERY_MODE PRE_TEST DISCOVERY_TIMEOUT 120)
