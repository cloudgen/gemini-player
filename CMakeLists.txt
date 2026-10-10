# Two targets, one file.
# Windows: gemini-player.exe (Win32 + WebView2). See requirement-cpp-windows-environment.md.
# Ubuntu: gemini-wilgat (GTK 3 + WebKitGTK). See requirement-cpp-dual-build.md.

cmake_minimum_required(VERSION 3.20)
project(gemini-player VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
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
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PLAYER REQUIRED
        gtk+-3.0
        webkit2gtk-4.1
        glib-2.0
        gobject-2.0
        gio-2.0
        sqlite3
        lua5.3
    )
    find_package(Threads REQUIRED)
    add_executable(gemini-wilgat
        src/main/cpp/auto_follow.cpp
        src/main/cpp/config.cpp
        src/main/cpp/cookies.cpp
        src/main/cpp/dialog_about.cpp
        src/main/cpp/dialog_version.cpp
        src/main/cpp/load_page_ogin_check.cpp
        src/main/cpp/login_detector_app.cpp
        src/main/cpp/log_commands.cpp
        src/main/cpp/log_format.cpp
        src/main/cpp/log_view_html.cpp
        src/main/cpp/lua_script_commands.cpp
        src/main/cpp/main.cpp
        src/main/cpp/menu_bookmark.cpp
        src/main/cpp/menu_bookmark_app.cpp
        src/main/cpp/menu_exit.cpp
        src/main/cpp/menu_log.cpp
        src/main/cpp/menu_lua_script.cpp
        src/main/cpp/menu_user_agent.cpp
        src/main/cpp/output.cpp
        src/main/cpp/path_utils.cpp
        src/main/cpp/settings.cpp
        src/main/cpp/ui_menu_app.cpp
        src/main/cpp/ui_window_app.cpp
        src/main/cpp/url_bar_go_button.cpp
        src/main/cpp/url_bar_page.cpp
        src/main/cpp/url_bar_user_agent.cpp
        src/main/cpp/webkit_setup.cpp
        src/main/cpp/userscript_manager.cpp
    )
    target_include_directories(gemini-wilgat PRIVATE
        ${PLAYER_INCLUDE_DIRS}
        /usr/local/include
    )
    target_compile_options(gemini-wilgat PRIVATE ${PLAYER_CFLAGS_OTHER} -Wall -Wextra)
    target_link_directories(gemini-wilgat PRIVATE ${PLAYER_LIBRARY_DIRS})
    target_link_libraries(gemini-wilgat PRIVATE ${PLAYER_LIBRARIES} Threads::Threads)
    target_link_options(gemini-wilgat PRIVATE ${PLAYER_LDFLAGS_OTHER})
endif()
