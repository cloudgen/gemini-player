**file**: docs/requirements/requirement-cpp-windows-environment.md
**Status**: Active
**Updated**: 2026-10-10
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the Windows C++ toolchain for gemini-player. It is the record of the machine `leolio@win` and of the tools the Windows build is allowed to use. The Linux link set stays in `requirement-cpp-dependencies.md`. The window behavior of the Windows executable stays in `requirement-cpp-windows-player.md`.

### 1.1 Human-facing

**In one sentence:** You compile gemini-player on Windows with the Visual Studio 2022 C++ tools that are already installed, and you do not install GTK or WebKit.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The Windows account that builds | `leolio` on `win` |
| The other role | The MSVC compiler | `cl.exe` from Visual Studio 2022 |
| Not this file | Which page the window opens | `requirement-cpp-windows-player.md` |

| Includes | Excludes |
|----------|----------|
| The compiler, the SDK, CMake, WebView2, and SQLite | Apt packages and WebKitGTK |

| Surface | What you open | What for |
|---------|---------------|----------|
| `build-win.bat` | Visual Studio developer environment, then CMake | The Windows build |
| `CMakeLists.txt` | The same compile, as the CMake description | `gemini-player.exe` |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Build on the Windows machine | x64 Debug, Visual Studio 17 2022 generator | `build-win.bat` |

## 2. Core rules

The Windows build host that was probed on 2026-10-10:

| Fact | Value |
|------|--------|
| SSH | `leolio@win` |
| Address | `192.168.2.232` |
| Computer | `LAPTOP-V8BRD791` |
| OS | Windows `10.0.26200` (`Windows_NT`, AMD64) |
| Login profile | `C:\Users\leolio` |
| Tree on that machine | `C:\Users\leolio\prjs\gemini-player` |

Compiler and SDK present on that machine. Versions are pins for this requirement. A newer Visual Studio 2022 update may replace the MSVC toolset. The build must still be the Visual Studio 17 2022 generator and x64.

| Tool | Location or version |
|------|---------------------|
| Visual Studio | Community 2022, `17.7.5` (`17.7.34202.233`) |
| Installation | `C:\Program Files\Microsoft Visual Studio\2022\Community` |
| Product id | `Microsoft.VisualStudio.Product.Community` |
| MSVC | `14.37.32822` under `VC\Tools\MSVC` |
| C++ workload | `Microsoft.VisualStudio.Component.VC.Tools.x86.x64` (vswhere returned this install when that component was required) |
| Developer script | `VC\Auxiliary\Build\vcvars64.bat` |
| Windows SDK | `10.0.22621.0` (also present: `10.0.22000.0`). The build uses the newest installed SDK |
| CMake | `3.26.4-msvc4`, bundled, not on `PATH` |
| CMake path | `Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` |
| Git | `C:\Program Files\Git\mingw64\bin\git.exe` |
| WebView2 Evergreen runtime | `154.0.4258.62`, client `{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}` |

`build-win.bat` calls `vcvars64.bat`, then that bundled `cmake.exe`. It configures with `-G "Visual Studio 17 2022" -A x64` and builds `--config Debug`. The output is `build-win\Debug\gemini-player.exe`.

Language and flags:

| Item | Value |
|------|--------|
| C++ standard | C++17 (`CMAKE_CXX_STANDARD 17`, `/std:c++17`) |
| C files | The SQLite amalgamation is C, compiled by the same MSVC toolset |
| Warnings | `/W3`. The SQLite amalgamation is compiled with `/w` |
| Exceptions | `/EHsc` |
| Source encoding | `/utf-8` |
| Character set | `UNICODE` and `_UNICODE` |
| Subsystem | Console, so `help` and `--check` print on the console. The process still opens a window |

Libraries the Windows link line is allowed to name:

| Library | Role |
|---------|------|
| `third_party/webview2/x64/WebView2Loader.dll.lib` | Import library for the WebView2 loader |
| `WebView2Loader.dll` | Copied next to `gemini-player.exe` by the CMake post-build step |
| `user32` `gdi32` | Window and controls |
| `ole32` `oleaut32` | COM for WebView2 |
| `shell32` | Open a folder from the menu |
| `shlwapi` `advapi32` `version` | System helpers required by the loader |

Vendored sources, committed so the Windows build does not download them again:

| Tree | Origin |
|------|--------|
| `third_party/webview2/include/WebView2.h` | NuGet `Microsoft.Web.WebView2` `1.0.4258.31` |
| `third_party/webview2/include/WebView2EnvironmentOptions.h` | Same package. Target product version in that header is `154.0.4258.31` |
| `third_party/webview2/x64/WebView2Loader.dll` and `WebView2Loader.dll.lib` | Same package, x64 |
| `third_party/sqlite/sqlite3.c` `sqlite3.h` `sqlite3ext.h` | SQLite amalgamation `3540000` (`3.54.0`) from `https://www.sqlite.org/2026/sqlite-amalgamation-3540000.zip` |

The loader DLL is the link choice. `WebView2LoaderStatic.lib` from the same package is about 11 MB and is not on the link line.

These are not Windows requirements. Do not add them to satisfy this file:

| Not required | Why |
|--------------|-----|
| GTK 3, GLib, WebKitGTK 4.1 | No MSVC port of WebKitGTK 4.1. `requirement-cpp-dependencies.md` still owns the Linux link set |
| MSYS2, vcpkg, NuGet CLI, Ninja on `PATH` | The probed machine does not have them on `PATH`, and this build does not need them |
| A second Visual Studio edition | Community 2022 is the installed product |
| An elevated prompt | The build and the player run as `leolio` |

The Linux commands `./build.sh` and `meson compile` stay the Linux build. They are not how this Windows tree is compiled. `CMakeLists.txt` stops on a non-Windows host and tells you to run `./build.sh`.

### 2.1 Why this requirement exists

- **CIAO Principle 5**: one Windows toolchain list. `build-win.bat` and `CMakeLists.txt` both consume this table.
- **CIAO Principle 1**: a missing `vcvars64.bat`, a missing bundled CMake, or a missing WebView2 runtime fails with the name of the missing piece.

## 3. Design principles

- **Caution**: the versions above were read from `leolio@win` with `vswhere`, the Windows SDK include directory, and the WebView2 client registry value `pv`.
- **Intentional**: WebView2 is the browser engine because that is the engine Visual Studio 2022 can link. GTK stays on Linux.
- **Anti-fragile**: headers and the loader DLL are in `third_party/`, so a later build does not depend on NuGet being reachable.
- **Over-protect**: the generator is Visual Studio 17 2022 x64. A Ninja generator is not a substitute on this machine.

## 4. Verification

| TP-ID | Pass |
|-------|------|
| TP-WIN-ENV | On `leolio@win`, `build-win.bat` exits 0 and `build-win\Debug\gemini-player.exe` exists beside `WebView2Loader.dll` |
| TP-WIN-CHECK | `gemini-player.exe --check` exits 0 and the console contains `check passed` |

Map: `docs/reviews/test-plan.md`.

## 5. Protection rule

Future editors must not point this build at WebKitGTK, replace the Visual Studio 17 2022 x64 generator with a MinGW build, or drop `WebView2Loader.dll` from the directory that contains `gemini-player.exe`.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-windows-player.md` | What the Windows executable does |
| `docs/requirements/requirement-cpp-dependencies.md` | Linux link set |
| `build-win.bat` | The command you run |
| `CMakeLists.txt` | The compile description |
| `third_party/webview2/` | WebView2 SDK files |
| `third_party/sqlite/` | SQLite amalgamation |

**Last updated**: 2026-10-10
**Owner**: Windows C++ environment
