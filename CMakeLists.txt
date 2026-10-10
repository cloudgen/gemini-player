# Windows player. Linux keeps ./build.sh and meson.build.
# Toolchain law: docs/requirements/requirement-cpp-windows-environment.md

cmake_minimum_required(VERSION 3.20)
project(gemini-player VERSION 1.0.7 LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This CMake build is the Windows player. On Linux run ./build.sh")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(gemini-player
    src/win/win_log.cpp
    src/win/win_host.cpp
    src/win/win_app.cpp
    third_party/sqlite/sqlite3.c
)

target_include_directories(gemini-player PRIVATE
    src/win
    src/main/cpp
    third_party/sqlite
    third_party/webview2/include
)

target_compile_definitions(gemini-player PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    SQLITE_THREADSAFE=1
    SQLITE_OMIT_LOAD_EXTENSION
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(gemini-player PRIVATE /W3 /EHsc /utf-8)
    set_source_files_properties(third_party/sqlite/sqlite3.c PROPERTIES COMPILE_OPTIONS "/w")
endif()

target_link_libraries(gemini-player PRIVATE
    "${CMAKE_SOURCE_DIR}/third_party/webview2/x64/WebView2Loader.dll.lib"
    user32
    gdi32
    ole32
    oleaut32
    shell32
    shlwapi
    advapi32
    version
)

add_custom_command(TARGET gemini-player POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/webview2/x64/WebView2Loader.dll"
        "$<TARGET_FILE_DIR:gemini-player>/WebView2Loader.dll"
    VERBATIM
)
