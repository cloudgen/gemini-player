# meson.build — second build for gemini-wilgat.
# Compiles the same C++ sources as ./build.sh.
# project() version matches the runtime macros in src/main/cpp/project.h (1.0.7).
# project.ini VERSION and the README badge remain 1.0.6 until those files are aligned.

project('gemini-wilgat', 'cpp',
    version: '1.0.7',
    license: 'MIT',
    default_options: [
        'warning_level=3',
        'buildtype=debugoptimized',
        'cpp_std=c++17',
    ]
)

gtk_dep = dependency('gtk+-3.0', required: true)
webkit_dep = dependency('webkit2gtk-4.1', required: true)
glib_dep = dependency('glib-2.0', required: true)
gobject_dep = dependency('gobject-2.0', required: true)
gio_dep = dependency('gio-2.0', required: true)
sqlite_dep = dependency('sqlite3', required: true)
lua_dep = dependency('lua5.3', required: true)
thread_dep = dependency('threads', required: true)

sol_inc = include_directories('/usr/local/include')

src_files = files(
    'src/main/cpp/auto_follow.cpp',
    'src/main/cpp/config.cpp',
    'src/main/cpp/cookies.cpp',
    'src/main/cpp/dialog_about.cpp',
    'src/main/cpp/dialog_version.cpp',
    'src/main/cpp/load_page_ogin_check.cpp',
    'src/main/cpp/login_detector_app.cpp',
    'src/main/cpp/log_commands.cpp',
    'src/main/cpp/log_format.cpp',
    'src/main/cpp/log_view_html.cpp',
    'src/main/cpp/lua_script_commands.cpp',
    'src/main/cpp/main.cpp',
    'src/main/cpp/menu_bookmark.cpp',
    'src/main/cpp/menu_bookmark_app.cpp',
    'src/main/cpp/menu_exit.cpp',
    'src/main/cpp/menu_log.cpp',
    'src/main/cpp/menu_lua_script.cpp',
    'src/main/cpp/menu_user_agent.cpp',
    'src/main/cpp/output.cpp',
    'src/main/cpp/path_utils.cpp',
    'src/main/cpp/settings.cpp',
    'src/main/cpp/ui_menu_app.cpp',
    'src/main/cpp/ui_window_app.cpp',
    'src/main/cpp/url_bar_go_button.cpp',
    'src/main/cpp/url_bar_page.cpp',
    'src/main/cpp/url_bar_user_agent.cpp',
    'src/main/cpp/webkit_setup.cpp',
    'src/main/cpp/userscript_manager.cpp',
)

executable('gemini-wilgat',
    sources: src_files,
    include_directories: sol_inc,
    dependencies: [
        gtk_dep,
        webkit_dep,
        glib_dep,
        gobject_dep,
        gio_dep,
        sqlite_dep,
        lua_dep,
        thread_dep,
    ],
    install: true,
    install_dir: get_option('bindir')
)
