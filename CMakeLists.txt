cmake_minimum_required(VERSION 3.25)
# A library a target reaches twice is kept on the link line only where the linker needs it (CMake 3.29 on).
if(POLICY CMP0156)
    cmake_policy(SET CMP0156 NEW)
endif()
# URL archives take the extraction time, so a changed dependency is newer than what was built from the one before.
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Reckless Sudoku: a Sudoku solved by deduction, a step at a time, as a person solves it. The solver (src/solver) needs
# nothing else; the window (src/app) is drawn with Skia on reckless-toolkit, a git submodule at
# external/reckless-toolkit.

# The oldest macOS the Mac binary runs on, set before project(): 13.3, the floor every Reckless product shares.
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "The oldest macOS the Mac binary runs on" FORCE)
endif()

# The version, MAJOR.MINOR.BUILD, from the nearest vMAJOR.MINOR tag and the commits since it (cmake/sudoku_version.cmake).
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(sudoku_version)

project(RecklessSudoku VERSION ${SUDOKU_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # the editor's code model reads build/<preset>/compile_commands.json

# cl and clang-cl both take MSVC's command line; every flag below is spelled for the front end, not the compiler.
set(SUDOKU_MSVC_FRONTEND OFF)
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(SUDOKU_MSVC_FRONTEND ON)
endif()

if(MSVC)
    # The static release runtime in every configuration: nothing to install beside the program, and the toolkit's
    # prebuilt Skia is built for it alone.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_compile_options(/utf-8)
endif()

option(SUDOKU_BUILD_TESTS "Build the test suites" ON)
option(SUDOKU_GUI "Build the window on the toolkit submodule (external/reckless-toolkit)" ON)
option(SUDOKU_PUZZLE_BANK "Fetch the Sudoku Exchange puzzle bank and put it beside the app, which offers new puzzles from it"
       ON)
option(SUDOKU_COVERAGE "Instrument the program's own code for coverage (gcc, clang or clang-cl)" OFF)
option(SUDOKU_SANITIZE "Build everything with AddressSanitizer and UndefinedBehaviorSanitizer (gcc or clang)" OFF)
option(SUDOKU_TSAN "Build everything with ThreadSanitizer (gcc or clang)" OFF)

# The sanitizers on every target, the toolkit's and GoogleTest's included, so what passes between them is checked on
# both sides. UndefinedBehaviorSanitizer's vptr check reads the type information of every polymorphic class a checked
# call reaches, and the window's prebuilt Skia is built without it (-fno-rtti): that one check is left out, the rest
# kept. ThreadSanitizer cannot share a process with AddressSanitizer, so it is a build of its own; it sees only what was
# compiled under it - the program and the toolkit, not the prebuilt Skia, which draws on the calling thread alone.
if(SUDOKU_SANITIZE AND SUDOKU_TSAN)
    message(FATAL_ERROR "SUDOKU_SANITIZE and SUDOKU_TSAN are separate builds: ThreadSanitizer cannot run beside "
                        "AddressSanitizer")
endif()
if((SUDOKU_SANITIZE OR SUDOKU_TSAN) AND SUDOKU_MSVC_FRONTEND)
    message(FATAL_ERROR "SUDOKU_SANITIZE and SUDOKU_TSAN are for gcc and clang")
endif()
if(SUDOKU_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-sanitize=vptr -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined -fno-sanitize=vptr)
endif()
if(SUDOKU_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

# The strict warning set on every compiler, for the program's own targets. Not errors: a new compiler must not break the
# build; the code is kept warning-free on gcc, clang and MSVC, and CI's logs are the check.
add_library(sudoku_warnings INTERFACE)
if(SUDOKU_MSVC_FRONTEND)
    target_compile_options(sudoku_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(sudoku_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Coverage of the program's own code alone: the flags ride on sudoku_warnings, which every target of it links and
# neither the toolkit nor GoogleTest does. clang and clang-cl count with LLVM's source-based coverage, read by llvm-cov
# (tools/coverage-windows.ps1 on Windows); clang-cl links through the MSVC linker, which takes no compiler flag, so the
# objects name clang's profile runtime themselves and the linker is shown where it lies.
if(SUDOKU_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(sudoku_warnings INTERFACE --coverage -O0)
        target_link_options(sudoku_warnings INTERFACE --coverage)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND SUDOKU_MSVC_FRONTEND)
        target_compile_options(sudoku_warnings INTERFACE -fprofile-instr-generate -fcoverage-mapping /Od)
        execute_process(COMMAND "${CMAKE_CXX_COMPILER}" /clang:-print-resource-dir
                        OUTPUT_VARIABLE clang_resource_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
        file(TO_CMAKE_PATH "${clang_resource_dir}" clang_resource_dir)
        if(NOT EXISTS "${clang_resource_dir}/lib/windows")
            message(FATAL_ERROR "clang-cl's profile runtime is not under ${clang_resource_dir}/lib/windows")
        endif()
        target_link_directories(sudoku_warnings INTERFACE "${clang_resource_dir}/lib/windows")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(sudoku_warnings INTERFACE -fprofile-instr-generate -fcoverage-mapping -O0)
        target_link_options(sudoku_warnings INTERFACE -fprofile-instr-generate)
    else()
        message(FATAL_ERROR "SUDOKU_COVERAGE needs gcc, clang or clang-cl, not ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# The version every binary reports, from project() above.
configure_file(cmake/version.h.in "${PROJECT_BINARY_DIR}/generated/version.h" @ONLY)

if(SUDOKU_BUILD_TESTS)
    enable_testing()
    # GoogleTest, pinned by its release tarball's hash.
    include(FetchContent)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL      https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    # googletest compiles its own two libraries under its own strict set (/WX on clang-cl), where clang 21's
    # -Wcharacter-conversion meets its char8_t printer in gtest-printers.h: googletest's to fix, so those two let it pass.
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(-Wcharacter-conversion SUDOKU_CLANG_KNOWS_CHARACTER_CONVERSION)
        if(SUDOKU_CLANG_KNOWS_CHARACTER_CONVERSION)
            target_compile_options(gtest PRIVATE -Wno-character-conversion)
            target_compile_options(gtest_main PRIVATE -Wno-character-conversion)
        endif()
    endif()
endif()

if(SUDOKU_GUI)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/reckless-toolkit/CMakeLists.txt")
        message(FATAL_ERROR "The window is built on the toolkit, a git submodule at external/reckless-toolkit, which this "
                            "checkout does not hold. Fetch it with 'git submodule update --init', or build the solver "
                            "and its suites alone with -DSUDOKU_GUI=OFF.")
    endif()
    # The toolkit's options, as normal variables before it is added: its windowed parts on, its own suites off, and no
    # audio device layer - the program makes no sound.
    set(RECKLESS_TOOLKIT_BUILD_TESTS OFF)
    set(RECKLESS_TOOLKIT_GUI ON)
    set(RECKLESS_BUILD_AUDIO OFF)
    set(RECKLESS_ASIO OFF)
    add_subdirectory(external/reckless-toolkit EXCLUDE_FROM_ALL)
endif()

add_subdirectory(src)
if(SUDOKU_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# THIRD-PARTY-NOTICES.txt as tools/third_party_notices.py writes it from this tree's fetches and the toolkit's pins: a
# pin that moves without the notices written again fails here. Any Python 3 runs it.
if(SUDOKU_BUILD_TESTS AND SUDOKU_GUI)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME notices
                 COMMAND Python3::Interpreter -I "${PROJECT_SOURCE_DIR}/tools/third_party_notices.py" "${CMAKE_BINARY_DIR}"
                         --check)
    else()
        message(WARNING "No Python 3 found: the test 'notices', which holds THIRD-PARTY-NOTICES.txt to the build's pins, "
                        "is left out")
    endif()
endif()

