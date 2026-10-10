**file**: docs/requirements/requirement-cpp-dual-build.md
**Status**: Active
**Updated**: 2026-10-10
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the rule that one tree builds and runs on Ubuntu and on Windows. The two programs stay separate. Ubuntu compiles `src/main/cpp` into `gemini-wilgat` with GTK 3 and WebKitGTK 4.1. Windows compiles `src/win` into `gemini-player.exe` with Win32 and WebView2. Neither host compiles the other host's engine.

### 1.1 Human-facing

**In one sentence:** You build this repository on Ubuntu and on Windows, and each host produces its own program.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person compiling on that host | Ubuntu here, or `leolio` on `win` |
| The other role | The build entry for that host | `./build.sh build` or `build-win.bat` |
| Not this file | Menu items and the start page | `requirement-cpp-window.md` and `requirement-cpp-windows-player.md` |

| Includes | Excludes |
|----------|----------|
| Which command builds which binary on which host | Merging WebKitGTK into the MSVC link |

| Surface | What you open | What for |
|---------|---------------|----------|
| `./build.sh` | Ubuntu GTK compile | `target/exe/debug/gemini-wilgat` |
| `build-win.bat` | Windows WebView2 compile | `build-win\Debug\gemini-player.exe` |
| `build.xml` and `build.gradle` | The same choice, driven by the host OS | `ant build`, `gradle assemble` |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build on Ubuntu | GTK program | `./build.sh build` |
| Build on Windows | WebView2 program | `build-win.bat` |
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

`PRJ` in `project.ini` stays `gemini-wilgat`. That is the Ubuntu binary name. The Windows executable name stays `gemini-player`.

`./build.sh` still has only `install`, `clean`, and `build`. It does not grow `ant` or `gradle` verbs. `requirement-cpp-skills-installation-ant.md` and `requirement-cpp-skills-installation-gradle.md` stay Deferred for that reason. Direct `ant` and `gradle` commands are this file's job.

### 2.2 What must not happen

- Do not compile `src/main/cpp` with MSVC.
- Do not link WebKitGTK into `gemini-player.exe`.
- Do not rename the Ubuntu binary to `gemini-player`.
- Do not put user-agent string literals anywhere except `src/main/cpp/project.h`.
- Do not rewrite the GTK `pkg-config` lines inside `build.sh` `build()`. A host check may return before those lines. The lines themselves stay.

### 2.3 Run

Ubuntu run proof is `target/exe/debug/gemini-wilgat --shutdown=N` on a graphical session. The process opens the GTK window, loads the start page, and exits when the timer fires.

Windows compile proof from SSH is `build-win.bat` then `gemini-player.exe --check`. A visible WebView2 window needs an interactive desktop. SSH itself is not that desktop.

## 3. Protection rule

Future editors must not collapse these two programs into one source list or one link line. A change that makes Ubuntu call `build-win.bat`, or Windows call `pkg-config` for GTK, fails this file.

## 4. Related artifacts

| Artifact | Role |
|----------|------|
| `build.sh` | Ubuntu GTK compile; Windows shell dispatch |
| `build.xml` | Ant host dispatch |
| `build.gradle` | Gradle host dispatch |
| `CMakeLists.txt` | Windows WebView2 target and Ubuntu GTK target |
| `meson.build` | Second Ubuntu compile |
| `requirement-cpp-dependencies.md` | Ubuntu libraries |
| `requirement-cpp-windows-environment.md` | Windows toolchain |
| `requirement-cpp-windows-player.md` | Windows window |
