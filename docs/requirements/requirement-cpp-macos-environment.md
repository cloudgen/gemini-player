**file**: docs/requirements/requirement-cpp-macos-environment.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the macOS C++ toolchain for gemini-player. It names the binaries the macOS build is allowed to run, and the zsh file that supplies PATH. The Linux link set stays in `requirement-cpp-dependencies.md`. The window behavior of the macOS executable stays in `requirement-cpp-macos-player.md`.

### 1.1 Human-facing

**In one sentence:** You compile gemini-player on macOS with zsh, clang, CMake, and make from `/usr/local/bin`, and PATH comes from the zshenv file.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The macOS account that builds | The account whose zshenv file is sourced |
| The other role | The compilers | `/usr/local/bin/clang` and `/usr/local/bin/clang++` |
| Not this file | Which page the window opens | `requirement-cpp-macos-player.md` |

| Includes | Excludes |
|----------|----------|
| `/usr/local/bin` tools, the zshenv PATH file, CMake, and the WebKit frameworks | `/usr/bin` tool names, `/bin/bash`, and WebKitGTK |

| Surface | What you open | What for |
|---------|---------------|----------|
| `build-mac.zsh` | zsh, then CMake | The macOS build |
| `CMakeLists.txt` | The same compile, as the CMake description | `build-mac/gemini-player` |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build on the Mac | Debug, Unix Makefiles, compilers under `/usr/local/bin` | `/usr/local/bin/zsh ./build-mac.zsh` |

## 2. Core rules

The script's own commands are these absolute paths:

| Tool | Path |
|------|------|
| zsh | `/usr/local/bin/zsh` |
| clang | `/usr/local/bin/clang` |
| clang++ | `/usr/local/bin/clang++` |
| CMake | `/usr/local/bin/cmake` |
| make | `/usr/local/bin/make` |

`build-mac.zsh` starts with `#!/usr/local/bin/zsh`. It refuses to continue when it is not running under zsh. Ant, Gradle, and `./build.sh` on Darwin invoke that same `/usr/local/bin/zsh` binary. They do not invoke `/bin/bash` for the macOS build.

PATH is read by sourcing `${ZSHENV:-$HOME/.zshenv}` before the tool check. That file is the zsh file sourced for every zsh process, including `zsh -c`. The build does not read `~/.bashrc` and does not start `/bin/bash` to obtain PATH. If the zshenv file is missing, the script exits 1 and names that file.

Each of the five binaries must be executable. They may be real files or symlinks that the operator placed in `/usr/local/bin`. The script does not create those symlinks. It does not call `/usr/bin/clang`, `/usr/bin/bash`, `/usr/bin/make`, or `xcrun`. It does not search `/opt/homebrew/bin` or `/usr/bin`.

CMake is invoked as `/usr/local/bin/cmake` with the Unix Makefiles generator, Debug, and these cache values:

| Cache value | Path |
|-------------|------|
| `CMAKE_C_COMPILER` | `/usr/local/bin/clang` |
| `CMAKE_CXX_COMPILER` | `/usr/local/bin/clang++` |
| `CMAKE_OBJC_COMPILER` | `/usr/local/bin/clang` |
| `CMAKE_OBJCXX_COMPILER` | `/usr/local/bin/clang++` |
| `CMAKE_MAKE_PROGRAM` | `/usr/local/bin/make` |

The build directory is `build-mac`. The output is `build-mac/gemini-player`.

Compiler and SDK versions are not pinned. On 2026-10-11 the macOS build host did not answer, so this file does not record a clang version, an Xcode version, or an SDK version. A later probe may add those pins. Until then the rule is the path, not a version number.

Language and flags, from `CMakeLists.txt` when `APPLE` is true:

| Item | Value |
|------|--------|
| C++ standard | C++17 |
| Objective-C++ standard | C++17 |
| Objective-C++ flags | `-fobjc-arc` `-Wall` `-Wextra` |
| C++ flags | `-Wall` `-Wextra` |
| C files | The SQLite amalgamation, compiled by `/usr/local/bin/clang` with warnings suppressed (`-w`) |
| Frameworks | `Cocoa` and `WebKit` |

Vendored SQLite is the same tree the Windows build uses: `third_party/sqlite/sqlite3.c`, `sqlite3.h`, and `sqlite3ext.h` (amalgamation `3.54.0`). The macOS build does not download it again. WebKit and Cocoa come from the system. They are not copied into `third_party/`.

These are not macOS requirements. Do not add them to satisfy this file:

| Not required | Why |
|--------------|-----|
| GTK 3, GLib, WebKitGTK 4.1, Lua, sol2 | `requirement-cpp-dependencies.md` still owns the Linux link set. `src/main/cpp` is not in the macOS binary |
| Meson | Meson stays the second Ubuntu compile |
| `/bin/bash` or `~/.bashrc` | PATH for this build comes from the zshenv file |
| A search of `/usr/bin` or `/opt/homebrew/bin` | The five tools above are absolute paths |

`./build.sh` on Darwin prints a one-line notice and runs `/usr/local/bin/zsh ./build-mac.zsh`, then returns before the GTK `pkg-config` lines. Those GTK lines stay. On Ubuntu, `./build.sh` still has the `#!/bin/bash` shebang and still compiles `gemini-wilgat`.

## 3. Design principles

- **Caution**: a missing zshenv file or a missing binary under `/usr/local/bin` exits 1 and names the missing piece.
- **Intentional**: WKWebView is the browser engine because that is the engine the macOS WebKit framework provides. GTK stays on Linux. WebView2 stays on Windows.
- **Anti-fragile**: the script names compilers by absolute path, so a `PATH` that lists `/usr/bin` first cannot select `/usr/bin/clang`.
- **Over-protect**: `CMAKE_MAKE_PROGRAM` is `/usr/local/bin/make`, so the generated Makefiles do not call `/usr/bin/make`.

## 4. Verification

| TP-ID | Pass |
|-------|------|
| TP-MAC-ENV | On macOS, `build-mac.zsh` or `./build.sh build` exits 0 and `build-mac/gemini-player` exists. The script's compilers, CMake, make, and zsh are the `/usr/local/bin` paths in the table above, and the script sources the zshenv file |

Map: `docs/reviews/test-plan.md`. This row stays todo until that command has been run on a Mac.

## 5. Protection rule

Future editors must not point this build at WebKitGTK, replace `/usr/local/bin/zsh` with `/bin/bash`, or look up clang, CMake, or make on `PATH` instead of the absolute paths in this file.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-macos-player.md` | What the macOS executable does |
| `docs/requirements/requirement-cpp-dependencies.md` | Linux link set |
| `docs/requirements/requirement-cpp-dual-build.md` | Which host runs which command |
| `build-mac.zsh` | The command you run |
| `CMakeLists.txt` | The compile description |
| `third_party/sqlite/` | SQLite amalgamation |

**Last updated**: 2026-10-11
**Owner**: macOS C++ environment
