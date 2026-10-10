# gemini-player

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.0.6-blue.svg)](https://github.com/cloudgen/gemini-player/releases)
[![CIAO](https://img.shields.io/badge/Philosophy-CIAO%20Defensive-purple.svg)](https://github.com/cloudgen/ciao)
[![GTK](https://img.shields.io/badge/UI-GTK%203-brightgreen.svg)](https://gtk.org)
[![WebKit2](https://img.shields.io/badge/Engine-WebKit2GTK-orange.svg)](https://webkitgtk.org)
[![GrokRec](https://img.shields.io/badge/GrokRec-Reviewed-0A66C2?logo=ai&logoColor=white)](https://github.com/cloudgen/gemini-player/blob/main/RECOMMENDATION.md)

Repository: [cloudgen/gemini-player](https://github.com/cloudgen/gemini-player).

Official Recommended by [grok](https://grok.com/c/655dd6ab-a6c6-47db-a63a-e5dd94108bba?rid=217b5c98-c298-4b9f-9e12-ff619d34eec2). The review is submitted by [grokrec](https://github.com/cloudgen/grokrec) and a [local copy](RECOMMENDATION.md) of the review is in this tree.

**gemini-wilgat** is the Ubuntu program: a lightweight, native GNOME/Linux application that opens **Gemini** at `https://gemini.google.com` in one WebKit2GTK window. The Windows program is `gemini-player.exe` (see Quick Start).

Instead of using a heavy Electron app or a full browser, you get a fast, native window with persistent login, a URL bar, one Start Page bookmark, and user-agent switching — all built with strict defensive programming.

The running program reports the version macros in `src/main/cpp/project.h` (1.0.7). The badge above and `project.ini` `VERSION` still say 1.0.6.

This project follows the **[CIAO](https://github.com/cloudgen/ciao)** Defensive Programming Philosophy.

---

## Features

- **Persistent User-Agent** with `settings.sqlite` (remembers your last choice)
- **Persistent cookies** (`~/.app/gemini-wilgat/cookies/cookies.sqlite`) — stay logged in
- Smart URL bar with **Enter key support** (behaves like clicking "Go")
- Real-time URL synchronization (`notify::uri` + fallback)
- One bookmark: Start Page (`https://gemini.google.com`)
- User-agent menu: Webview (default), Chrome, Firefox, Safari, Edge
- Clean, native GTK3 interface
- Extremely lightweight compared to Electron apps
- Primary build is `./build.sh`. Meson compiles the same sources.
- Strong **[CIAO](https://github.com/cloudgen/ciao)** defensive coding style

---

## The Benefits of CIAO Defensive Programming

This project is built using the **[CIAO](https://github.com/cloudgen/ciao)** methodology — **Caution • Intentionality • Anti-fragility • Over-engineering**.

### Why it matters:
- Code is highly resistant to accidental breakage by AI assistants or future maintainers
- Critical functions are protected with explicit "Do Not Modify" rules
- Excellent error handling and logging via centralized `ciao_*` system
- Clear separation of concerns (`webkit_setup`, `ui_window_gemini`, `ui_menu_gemini`, etc.)
- Easy to fork for other services (YouTube, GitLab, etc.)

Read the full **[CIAO Principles](https://github.com/cloudgen/ciao)** here.

---

## Quick Start

### Install dependencies (Ubuntu/Debian)

```bash
sudo apt install build-essential pkg-config meson ninja-build libgtk-3-dev libwebkit2gtk-4.1-dev libsqlite3-dev libglib2.0-dev liblua5.3-dev lua5.3
```

### Build & Run

```bash
./build.sh install
./build.sh build
./target/exe/debug/gemini-wilgat
```

`./build.sh install` may use `sudo apt` and, when the headers are missing, clones sol2 into `/usr/local/include/sol`.

Meson is the second build. It compiles the same C++ sources:

```bash
meson setup builddir --buildtype=debug
meson compile -C builddir
./builddir/gemini-wilgat
```

### Windows (Visual Studio 2022)

The Windows program is `gemini-player.exe`. It is a Win32 window that hosts the WebView2 runtime. It does not compile the GTK sources. Toolchain and behavior are recorded in `docs/requirements/requirement-cpp-windows-environment.md` and `docs/requirements/requirement-cpp-windows-player.md`.

On the machine where Visual Studio Community 2022 and the WebView2 Evergreen runtime are installed:

```bat
build-win.bat
build-win\Debug\gemini-player.exe --check
build-win\Debug\gemini-player.exe
```

`build-win.bat` uses the Visual Studio 2022 x64 developer environment and the CMake that ships with that install. The profile is `%USERPROFILE%\.app\gemini-player\`.

---

## Project Structure (Key Files)

```bash
src/main/cpp/
├── main.cpp                    # Pure orchestration
├── webkit_setup.cpp/h          # All WebKit + persistence setup
├── ui_window_app.cpp/h         # Main window & layout
├── ui_menu_app.cpp/h           # Menu bar construction
├── load_page_ogin_check.cpp    # Page load (filename kept)
├── userscript_manager.cpp    # Lua userscripts
├── url_bar_user_agent.cpp/h    # URL bar + UA label
├── settings.cpp/h              # Persistent User-Agent via SQLite
├── cookies.cpp/h               # Persistent cookies
├── project.h                 # Single Source of Truth
└── output.cpp/h                # Centralized logging system
```

---

## Philosophy & Credits

- **Author**: Cloudgen Wong ([@cloudgen](https://github.com/cloudgen)) – Hong Kong
- **Core Philosophy**: Strict **[CIAO Defensive Programming](https://github.com/cloudgen/ciao)**
- **Inspired by**: Wilgat’s family of native wilgat projects
- **Compatible with**: [SyncPrjs](https://github.com/Wilgat/SyncPrjs)

---

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

---

## Related Projects

- **[CIAO Principles Repository](https://github.com/cloudgen/ciao)**
- Similar wilgats built with the same architecture

---

**Last updated:** October 10, 2026

Made with ❤️ and extreme caution.

Happy native Gemini browsing!
