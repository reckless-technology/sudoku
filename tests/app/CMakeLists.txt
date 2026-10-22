# The window's suite: the controller, the menus, the settings file and the views (painted into images, driven by
# events), with no window.
add_executable(sudoku_app_test controller_test.cpp settings_file_test.cpp views_test.cpp)
target_link_libraries(sudoku_app_test PRIVATE sudoku_gui GTest::gtest_main sudoku_warnings)
gtest_discover_tests(sudoku_app_test DISCOVERY_MODE PRE_TEST)
