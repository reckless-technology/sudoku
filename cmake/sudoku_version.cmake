# Reckless Sudoku's version, MAJOR.MINOR.BUILD, from git alone - the rule every Reckless project follows: MAJOR.MINOR
# is the nearest tag of the form vMAJOR.MINOR reachable from HEAD, made by hand (a release's own vMAJOR.MINOR.BUILD tags
# are three-component and excluded), and BUILD is the number of commits since it, the tagged commit itself being 0.
# Nothing in the tree is edited to release. Included before project(), which takes SUDOKU_VERSION as the project's
# version, so PROJECT_VERSION and its parts are this number everywhere after.
#
# A checkout with no such tag reachable (a shallow clone, a fork without the tags, or the repository before its first
# vMAJOR.MINOR tag) builds as 0.0.0 with a warning. The configure runs again when HEAD moves; after `git tag`,
# reconfigure by hand.
include_guard(GLOBAL)

find_program(SUDOKU_GIT_EXECUTABLE NAMES git DOC "The git that reads the version from the tags")
set(_sudoku_describe "")
set(_sudoku_result 1)
if(SUDOKU_GIT_EXECUTABLE)
    execute_process(
        COMMAND "${SUDOKU_GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" describe --tags --long
                --match "v[0-9]*.[0-9]*" --exclude "v*.*.*"
        OUTPUT_VARIABLE _sudoku_describe
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _sudoku_result
        ERROR_QUIET)
endif()
if(_sudoku_result EQUAL 0 AND _sudoku_describe MATCHES "^v([0-9]+)\\.([0-9]+)-([0-9]+)-g[0-9a-f]+$")
    set(SUDOKU_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
else()
    message(WARNING "No vMAJOR.MINOR tag is reachable from HEAD (git describe said '${_sudoku_describe}'): the build "
                    "reports 0.0.0. Fetch the tags (git fetch --tags; CI checks out with fetch-depth 0).")
    set(SUDOKU_VERSION "0.0.0")
endif()
message(STATUS "Reckless Sudoku ${SUDOKU_VERSION}")

# A new commit or a checkout configures again, so the number follows HEAD.
if(SUDOKU_GIT_EXECUTABLE)
    execute_process(
        COMMAND "${SUDOKU_GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse --absolute-git-dir
        OUTPUT_VARIABLE _sudoku_git_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    execute_process(
        COMMAND "${SUDOKU_GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse --path-format=absolute
                --git-common-dir
        OUTPUT_VARIABLE _sudoku_git_common_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    foreach(_sudoku_ref "${_sudoku_git_dir}/HEAD" "${_sudoku_git_dir}/logs/HEAD" "${_sudoku_git_common_dir}/packed-refs")
        if(EXISTS "${_sudoku_ref}")
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_sudoku_ref}")
        endif()
    endforeach()
endif()
