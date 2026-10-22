# The window, on the toolkit: the app (RecklessSudoku) - a Windows executable with its version resource and icon, a
# macOS .app, a Linux binary - the render tool, and the library both are made from, which the window's suite links.

# The files the binary carries (the toolkit's reckless_module_assets): the series' serif and the company mark, the
# monospaced family the pencil marks are set in, and the app's icon at 256 pixels for the window's title bar on Linux
# and the About card.
reckless_monospace_font_files(sudoku_monospace)
reckless_module_assets(sudoku_assets ${sudoku_monospace} "icon_256.png=${PROJECT_SOURCE_DIR}/assets/icon/icon_256.png")

add_library(sudoku_gui STATIC
    about_view.cpp
    board_view.cpp
    controller.cpp
    main_view.cpp
    menus.cpp
    settings_file.cpp
    side_panel.cpp)
target_include_directories(sudoku_gui PUBLIC "${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(sudoku_gui
    PUBLIC  sudoku_solver sudoku_assets reckless_app_shell reckless_app_resources reckless_common_panel
            reckless_gfx_skia reckless_core_preset_files
    PRIVATE sudoku_warnings)

add_executable(RecklessSudoku main.cpp)
target_link_libraries(RecklessSudoku PRIVATE sudoku_gui reckless_app_platform sudoku_warnings)
# On Linux the program is named as Linux programs are, in lower case.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(RecklessSudoku PROPERTIES OUTPUT_NAME reckless-sudoku)
endif()

# The render tool: the window drawn to a PNG in a chosen state, and the app's icon at any size.
add_executable(sudoku_render render_main.cpp)
target_link_libraries(sudoku_render PRIVATE sudoku_gui sudoku_warnings)

# The identity the executable carries, as the toolkit's bundle helpers read it.
set(SUDOKU_ICONS "${PROJECT_SOURCE_DIR}/assets/icon")
set(SUDOKU_COMPANY "Reckless Technology")
set(SUDOKU_DISPLAY "Reckless Sudoku")
set(SUDOKU_COPYRIGHT "Copyright (C) 2008-2026 Jonny Reckless")
set(RECKLESS_VERSION "${PROJECT_VERSION}")
set(RECKLESS_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(RECKLESS_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(RECKLESS_VERSION_BUILD "${PROJECT_VERSION_PATCH}")

if(APPLE)
    set(BUNDLE_EXECUTABLE RecklessSudoku)
    set(BUNDLE_IDENTIFIER com.recklesstechnology.sudoku)
    set(DISPLAY "${SUDOKU_DISPLAY}")
    set(VERSION "${PROJECT_VERSION}")
    set(COPYRIGHT "${SUDOKU_COPYRIGHT}")
    set(plist "${CMAKE_CURRENT_BINARY_DIR}/sudoku_info.plist")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/info.plist.in" "${plist}" @ONLY)
    set_target_properties(RecklessSudoku PROPERTIES MACOSX_BUNDLE TRUE MACOSX_BUNDLE_INFO_PLIST "${plist}")
    target_sources(RecklessSudoku PRIVATE "${SUDOKU_ICONS}/icon.icns")
    set_source_files_properties("${SUDOKU_ICONS}/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    reckless_sign_bundle(RecklessSudoku "$<TARGET_BUNDLE_DIR:RecklessSudoku>")
elseif(WIN32)
    # A window of its own, no console; main() as its entry, as the toolkit's platform expects.
    set_target_properties(RecklessSudoku PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        target_link_options(RecklessSudoku PRIVATE "/ENTRY:mainCRTStartup")
    endif()
    reckless_windows_version_resource(RecklessSudoku SUDOKU APP RecklessSudoku.exe "${SUDOKU_ICONS}/icon.ico")
endif()

if(SUDOKU_BUILD_TESTS)
    # The app's own launch: its window opened, a few frames drawn, closed.
    add_test(NAME app_smoke COMMAND RecklessSudoku --smoke)
    set_tests_properties(app_smoke PROPERTIES LABELS window RESOURCE_LOCK display TIMEOUT 120)

    # The render tool drawing every state it knows, and the icon, into the build tree: each must write its picture.
    foreach(state empty puzzle clue x-wing solved stuck clash no-marks about)
        add_test(NAME render_${state} COMMAND sudoku_render "${CMAKE_CURRENT_BINARY_DIR}/render_${state}.png" ${state})
    endforeach()
    add_test(NAME render_icon COMMAND sudoku_render "${CMAKE_CURRENT_BINARY_DIR}/render_icon.png" icon 64)
    # With no file to write, it says how it is used and fails.
    add_test(NAME render_usage COMMAND sudoku_render)
    set_tests_properties(render_usage PROPERTIES WILL_FAIL TRUE)
endif()
