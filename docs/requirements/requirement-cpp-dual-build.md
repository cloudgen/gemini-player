**file**: docs/requirements/requirement-cpp-dual-build.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the rule that one tree builds and runs on Ubuntu, on Windows, and on macOS. The three programs stay separate. Ubuntu compiles `src/main/cpp` into `gemini-wilgat` with GTK 3 and WebKitGTK 4.1. Windows compiles `src/win` into `gemini-player.exe` with Win32 and WebView2. macOS compiles `src/mac` into `gemini-player` with Cocoa and WKWebView. No host compiles another host's engine.

### 1.1 Human-facing

**In one sentence:** You build this repository on Ubuntu, on Windows, and on macOS, and each host produces its own program.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person compiling on that host | Ubuntu here, `leolio` on `win`, or the macOS login |
| The other role | The build entry for that host | `./build.sh build`, `build-win.bat`, or `build-mac.zsh` |
| Not this file | Menu items and the start page | `requirement-cpp-window.md`, `requirement-cpp-windows-player.md`, and `requirement-cpp-macos-player.md` |

| Includes | Excludes |
|----------|----------|
| Which command builds which binary on which host | Merging WebKitGTK into the MSVC link |

| Surface | What you open | What for |
|---------|---------------|----------|
| `./build.sh` | Ubuntu GTK compile, or a host dispatch | `target/exe/debug/gemini-wilgat` on Ubuntu |
| `build-win.bat` | Windows WebView2 compile | `build-win\Debug\gemini-player.exe` |
| `build-mac.zsh` | macOS WebKit compile | `build-mac/gemini-player` |
| `build.xml` and `build.gradle` | The same choice, driven by the host OS | `ant build`, `gradle assemble` |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build on Ubuntu | GTK program | `./build.sh build` |
| Build on Windows | WebView2 program | `build-win.bat` |
| Build on macOS | WebKit program | `/usr/local/bin/zsh ./build-mac.zsh` |
| Use Ant or Gradle | The host picks the engine | `ant build` or `gradle assemble` |

## 2. Core rules

### 2.1 Entry points

| Host | Command | Binary |
|------|---------|--------|
| Ubuntu | `./build.sh build` | `target/exe/debug/gemini-wilgat` |
| Ubuntu | `ant build` | `target/exe/debug/gemini-wilgat` |
| Ubuntu | `gradle assemble` | `target/exe/debug/gemini-wilgat` |
| Ubuntu | `meson compile -C builddir` | `builddir/gemini-wilgat` |
| Ubuntu | `cmake -S . -B build-linux && cmake --build build-linux` | `build-linux/gemini-wilgat` |
| Windows | `build-win.bat` | `build-win\Debug\gemini-player.exe` |
| Windows | `ant build` | calls `build-win.bat` |
| Windows | `gradle assemble` | calls `build-win.bat` |
| MSYS, MinGW, or Cygwin | `./build.sh build` | calls `build-win.bat` and returns before the GTK compile |
| macOS | `/usr/local/bin/zsh ./build-mac.zsh` | `build-mac/gemini-player` |
| macOS | `./build.sh build` | calls `/usr/local/bin/zsh ./build-mac.zsh` and returns before the GTK compile |
| macOS | `ant build` | calls `/usr/local/bin/zsh build-mac.zsh` |
| macOS | `gradle assemble` | calls `/usr/local/bin/zsh build-mac.zsh` |
| macOS | `cmake -S . -B build-mac` with the compilers named by `build-mac.zsh` | `build-mac/gemini-player` |

`PRJ` in `project.ini` stays `gemini-wilgat`. That is the Ubuntu binary name. The Windows executable name stays `gemini-player`. The macOS binary name is `gemini-player`, with no `.exe`. Meson stays the Ubuntu compile. It does not grow a macOS target.

`./build.sh` still has only `install`, `clean`, and `build`. It does not grow `ant` or `gradle` verbs. `requirement-cpp-skills-installation-ant.md` and `requirement-cpp-skills-installation-gradle.md` stay Deferred for that reason. Direct `ant` and `gradle` commands are this file's job.

### 2.2 What must not happen

- Do not compile `src/main/cpp` with MSVC.
- Do not compile `src/main/cpp` into `gemini-player` on macOS.
- Do not link WebKitGTK into `gemini-player.exe` or into the macOS `gemini-player`.
- Do not rename the Ubuntu binary to `gemini-player`.
- Do not put user-agent string literals anywhere except `src/main/cpp/project.h`.
- Do not rewrite the GTK `pkg-config` lines inside `build.sh` `build()`. A host check may return before those lines. The lines themselves stay.
- Do not make `build-mac.zsh` call `/bin/bash`, `/usr/bin/clang`, `/usr/bin/make`, or `xcrun`. The macOS tool paths are `requirement-cpp-macos-environment.md`.

### 2.3 Run

Ubuntu run proof is `target/exe/debug/gemini-wilgat --shutdown=N` on a graphical session. The process opens the GTK window, loads the start page, and exits when the timer fires.

Windows compile proof from SSH is `build-win.bat` then `gemini-player.exe --check`. A visible WebView2 window needs an interactive desktop. SSH itself is not that desktop.

macOS compile proof is `build-mac.zsh` then `build-mac/gemini-player --check`. `--check` does not open a window. A visible WKWebView needs a graphical session.

## 3. Protection rule

Future editors must not collapse these three programs into one source list or one link line. A change that makes Ubuntu call `build-win.bat` or `build-mac.zsh`, or that makes Windows or macOS call `pkg-config` for GTK, fails this file.

## 4. Related artifacts

| Artifact | Role |
|----------|------|
| `build.sh` | Ubuntu GTK compile; Windows and macOS shell dispatch |
| `build.xml` | Ant host dispatch |
| `build.gradle` | Gradle host dispatch |
| `build-mac.zsh` | macOS WebKit compile |
| `CMakeLists.txt` | Windows WebView2 target, macOS WebKit target, and Ubuntu GTK target |
| `meson.build` | Second Ubuntu compile |
| `requirement-cpp-dependencies.md` | Ubuntu libraries |
| `requirement-cpp-windows-environment.md` | Windows toolchain |
| `requirement-cpp-windows-player.md` | Windows window |
| `requirement-cpp-macos-environment.md` | macOS toolchain |
| `requirement-cpp-macos-player.md` | macOS window |

**Last updated**: 2026-10-11
**Owner**: Dual build
