**file**: docs/requirements/requirement-cpp-windows-player.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the Windows program `gemini-player.exe`. The Linux program remains `gemini-wilgat`, built from `src/main/cpp` by `./build.sh`. The Windows program is a separate source set. It opens Gemini in the WebView2 runtime instead of WebKitGTK.

### 1.1 Human-facing

**In one sentence:** You launch gemini-player on Windows and it opens Gemini in one window, with the address bar, the same five user-agent choices, and a profile that survives the next launch.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person at the Windows desktop | You double-click `gemini-player.exe` |
| The other role | The WebView2 window | It loads `https://gemini.google.com` |
| Not this file | Which compiler is installed | `requirement-cpp-windows-environment.md` |

| Includes | Excludes |
|----------|----------|
| Identity, profile paths, window, menus, user agent, cookies via the WebView2 profile | The Linux GTK call sequence |

| Surface | What you open | What for |
|---------|---------------|----------|
| `gemini-player.exe` | The player window | Gemini |
| `%USERPROFILE%\.app\gemini-player\` | The profile | Config, settings, WebView2 data, logs |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Open the start page | The default page is Gemini | launch the exe |
| Check the toolchain without a window | The profile and the WebView2 environment are created, then the process exits | `gemini-player.exe --check` |

## 2. Core rules

### 2.1 Identity

`src/win/player_project.h` includes `src/main/cpp/project.h` and then replaces only the product identity. User-agent string literals stay in `project.h`. Windows code must not paste those strings into another file.

| Macro | Windows value |
|-------|----------------|
| `PROJECT_ID` | `gemini-player` |
| `PROJECT_NAME` | `Gemini` |
| `PROJECT_DESCRIPTION` | `A lightweight native Gemini player built with WebView2 on Windows.` |
| `VERSION_MAJOR` `VERSION_MINOR` `VERSION_PATCH` | Unchanged from `project.h` (`1.2.0`) |
| `START_PAGE` | Unchanged from `project.h` (`https://gemini.google.com`) |
| `PLAYER_DEFAULT_USER_AGENT` | `UA_EDGE` |

`UA_EDGE` is the Windows default because the process embeds the Edge WebView2 runtime. `UA_WEBVIEW` remains on the User Agent menu. It is not the stored default on a first Windows run.

### 2.2 Profile

`USERPROFILE` is the home directory. If it is missing, `HOME` is used. `~` at the start of a config path expands to that directory. Slashes in an expanded path become backslashes.

```text
%USERPROFILE%\.app\gemini-player\
├── config.conf
├── settings.sqlite
├── cookies\                 created; WebView2 does not write cookies.sqlite here
├── webview2\                WebView2 user data folder (cookies, cache, login)
├── userscripts\
└── log\gemini-player-YYYYMMDD.log
```

`settings.sqlite` uses the same table as the Linux program: `settings(key TEXT PRIMARY KEY, value TEXT NOT NULL)`. The user-agent key is `SETTINGS_KEY_USER_AGENT` from `project.h`.

There is no separate root log directory. An elevated process still uses `%USERPROFILE%\.app\gemini-player\log`.

The first launch writes `config.conf` when that file is absent. The keys it writes match the Linux first-run file: `[core]`, `[ui]`, `[storage]`, `[output]`, `[bookmarks]`, `[scripts]`, `[parameters]`. `base_path` is `~/.app/gemini-player`.

### 2.3 Window

Top to bottom, when the matching config flag is true:

1. Menu bar.
2. Back, Forward, Reload, the URL edit, and Go.
3. The read-only user-agent label.
4. The WebView2 view, which takes the remaining space.

Default size is `CONFIG_DEFAULT_WINDOW_WIDTH` by `CONFIG_DEFAULT_WINDOW_HEIGHT` from `config.h` (1200 by 800). The process is per-monitor DPI aware.

Enter in the URL edit does the same thing as Go. A value with no `://` is opened as `https://` plus that value. The edit follows the view's source.

The page opened at startup is `START_PAGE`. `--bookmark=0` and a missing bookmark flag do that. Any other `--bookmark` value logs a warning and still opens `START_PAGE`. The Bookmark menu has one item, Start Page.

WebView2 is created with:

1. User data folder `%USERPROFILE%\.app\gemini-player\webview2`.
2. Additional browser argument `--autoplay-policy=no-user-gesture-required`.
3. JavaScript enabled, status bar hidden, devtools allowed.
4. The stored user agent applied before the first navigation.
5. Permission requests allowed.
6. `NewWindowRequested` opens a second window and hands that view back to WebView2, so a login popup is not dropped.

Cookies and the Google login live in the WebView2 user data folder. This build does not import or write `cookies.sqlite`.

### 2.4 Menus

File, when bookmarks are enabled: AutoClick all bots, Bookmark, User Agent, Lua Script, Log, Exit.

File, when bookmarks are disabled: User Agent, Lua Script, Log, Exit. AutoClick and Bookmark are absent.

About: Show Version. The dialog shows `PROJECT_NAME`, the version macros, and `PROJECT_DESCRIPTION`.

User Agent items, in order, using the macros in `project.h`: Webview (Default) `UA_WEBVIEW`, Google Chrome `UA_CHROME`, Mozilla Firefox `UA_FIREFOX`, Apple Safari `UA_SAFARI`, Microsoft Edge `UA_EDGE`. Choosing one saves `settings.sqlite`, updates the label, sets the WebView2 user agent, and reloads.

AutoClick all bots does not navigate. The Linux bot list is only the `NULL` sentinel, so the Windows item logs `[AUTOCLICK] Sequence finished or cancelled`.

Lua Script → Folder opens the userscripts directory. This build does not load sol2 and does not run `*.lua` files.

Log → Log folder opens the log directory. The stored line is:

```text
[YYYY-MM-DD HH:MM:SS] pid:<pid> [LEVEL] @<file> :] <message>
```

`LEVEL` is `INFO`, `WARNING`, `ERROR`, or `DEBUG`. The console prints `WARN` for a warning. `DEBUG` lines are stored and printed only when `DEBUG` is set to a non-empty value other than `0`, or when config `[output] debug` is true. `--quiet` hides info, warning, and debug on the console. Errors still print. `--json` forces quiet. The stored file is still appended.

### 2.5 Command line

| Argument | Effect |
|----------|--------|
| `help`, `--help`, `-h` | Print usage and exit 0. No window |
| `--check` | Create the profile, open `settings.sqlite`, create the WebView2 environment, print `check passed`, exit 0. No window. Exit 1 when the runtime or the environment is missing |
| `--shutdown=N` or `SHUTDOWN=N` | Close the window after N seconds when N is greater than 0 |
| `--bookmark=N` or `BOOKMARK=N` | See §2.3 |
| `--config PATH` | Read that file instead of the profile `config.conf` |
| `--quiet` or `QUIET=1` | Quiet console |
| `--json` or `JSON=1` | Quiet console |

A flag wins over the environment variable, and either one wins over `config.conf`.

### 2.6 What this Windows build does not do

These stay on the Linux program until a later requirement says otherwise:

- sol2 and Lua userscript execution
- the `log` and `luascript` command verbs
- the HTML log table inside the web view
- GStreamer environment variables
- compiling `src/main/cpp/*.cpp`

### 2.7 Why this requirement exists

- **CIAO Principle 5**: Windows behavior has one owner. Linux window law is not rewritten to pretend the GTK program is the Windows program.
- **CIAO Principle 1**: user-agent strings still have one definition, `project.h`.

## 3. Design principles

- **Caution**: the WebView2 profile directory is created before `CreateCoreWebView2EnvironmentWithOptions`.
- **Intentional**: the executable name and `PROJECT_ID` are `gemini-player`. The Linux binary name stays `gemini-wilgat`.
- **Anti-fragile**: `--check` proves the runtime without requiring the window to be visible over SSH.
- **Over-protect**: a new window request is given a real popup view. It is not ignored.

## 4. Verification

| TP-ID | Pass |
|-------|------|
| TP-WIN-CHECK | `--check` exits 0, prints `check passed`, and leaves `settings.sqlite` under `%USERPROFILE%\.app\gemini-player\` |
| TP-WIN-PLAYER | With `--shutdown=20`, the process starts, logs `WebView2 view ready`, and exits after the timer. The startup URL is the start page |

An SSH session on Windows is not the interactive desktop. `CreateCoreWebView2Controller` then returns `0x80070578` (`ERROR_INVALID_WINDOW_HANDLE`). `--check` does not create a window, and it is the remote proof that the runtime starts. The visible window is started from the signed-in session. A scheduled task with `/IT` is one way to do that from SSH when `leolio` is already logged on at the console.

## 5. Protection rule

Future editors must not copy user-agent string literals into `src/win`, must not point the WebView2 user data folder at a temporary directory that is deleted on exit, and must not compile the GTK sources into `gemini-player.exe`.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-windows-environment.md` | Toolchain |
| `docs/requirements/requirement-cpp-window.md` | Linux window |
| `src/win/player_project.h` | Windows identity |
| `src/win/win_app.cpp` | Window and WebView2 |
| `src/win/win_host.cpp` | Profile, config, settings |
| `src/win/win_log.cpp` | Stored log line |
| `src/main/cpp/project.h` | User-agent strings and version macros |

**Last updated**: 2026-10-11
**Owner**: Windows player
