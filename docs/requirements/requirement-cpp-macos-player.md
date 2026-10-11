**file**: docs/requirements/requirement-cpp-macos-player.md
**Status**: Active
**Updated**: 2026-10-11
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns the macOS program `gemini-player`. The Linux program remains `gemini-wilgat`, built from `src/main/cpp` by `./build.sh`. The Windows program remains `gemini-player.exe`, built from `src/win`. The macOS program is a third source set. It opens Gemini in WKWebView.

### 1.1 Human-facing

**In one sentence:** You launch gemini-player on macOS and it opens Gemini in one window, with the address bar, the same five user-agent choices, and a profile that survives the next launch.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person at the Mac | You start `build-mac/gemini-player` |
| The other role | The WKWebView window | It loads `https://gemini.google.com` |
| Not this file | Which compiler is installed | `requirement-cpp-macos-environment.md` |

| Includes | Excludes |
|----------|----------|
| Identity, profile paths, window, menus, user agent, login via the system WebKit data store | The Linux GTK call sequence and the Windows WebView2 call sequence |

| Surface | What you open | What for |
|---------|---------------|----------|
| `build-mac/gemini-player` | The player window | Gemini |
| `$HOME/.app/gemini-player/` | The profile | Config, settings, logs |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Open the start page | The default page is Gemini | launch the binary |
| Check the profile without a window | The profile and `settings.sqlite` are created, then the process exits | `build-mac/gemini-player --check` |

## 2. Core rules

### 2.1 Identity

`src/mac/player_project.h` includes `src/main/cpp/project.h` and then replaces only the product identity. User-agent string literals stay in `project.h`. macOS code must not paste those strings into another file.

| Macro | macOS value |
|-------|-------------|
| `PROJECT_ID` | `gemini-player` |
| `PROJECT_NAME` | `Gemini` |
| `PROJECT_DESCRIPTION` | `A lightweight native Gemini player built with WebKit on macOS.` |
| `VERSION_MAJOR` `VERSION_MINOR` `VERSION_PATCH` | Unchanged from `project.h` (`1.2.0`) |
| `START_PAGE` | Unchanged from `project.h` (`https://gemini.google.com`) |
| `PLAYER_DEFAULT_USER_AGENT` | `UA_SAFARI` |

`UA_SAFARI` is the macOS default because the process embeds WKWebView. `UA_WEBVIEW` remains on the User Agent menu. It is not the stored default on a first macOS run.

### 2.2 Profile

`HOME` is the home directory. The player logs an error when `HOME` is unset. `~` at the start of a config path expands to `HOME`.

```text
$HOME/.app/gemini-player/
├── config.conf
├── settings.sqlite
├── cookies/                 created; this build does not write cookies.sqlite here
├── webkit/                  created; this is not the WKWebView data store
├── userscripts/
└── log/gemini-player-YYYYMMDD.log
```

`settings.sqlite` uses the same table as the Linux program: `settings(key TEXT PRIMARY KEY, value TEXT NOT NULL)`. The user-agent key is `SETTINGS_KEY_USER_AGENT` from `project.h`.

There is no separate root log directory. The log directory is `$HOME/.app/gemini-player/log`.

The first launch writes `config.conf` when that file is absent. The keys it writes match the Linux first-run file: `[core]`, `[ui]`, `[storage]`, `[output]`, `[bookmarks]`, `[scripts]`, `[parameters]`. `base_path` is `~/.app/gemini-player`.

WKWebView has no public setting for a custom website-data folder in this program. Login and cookies use `[WKWebsiteDataStore defaultDataStore]`, which is the system WebKit store for this user. The `webkit/` directory and the `cookies/` directory are created so the profile layout stays recognizable. They are not the cookie database.

### 2.3 Window

Top to bottom, when the matching config flag is true:

1. Menu bar.
2. Back, Forward, Reload, the URL field, and Go.
3. The read-only user-agent label.
4. The WKWebView, which takes the remaining space.

Default size is `CONFIG_DEFAULT_WINDOW_WIDTH` by `CONFIG_DEFAULT_WINDOW_HEIGHT` from `config.h` (1200 by 800).

Enter in the URL field does the same thing as Go. A value with no `://` is opened as `https://` plus that value. The field follows the view after a finished navigation.

The page opened at startup is `START_PAGE`. `--bookmark=0` and a missing bookmark flag do that. Any other `--bookmark` value logs a warning and still opens `START_PAGE`. The Bookmark menu has one item, Start Page.

WKWebView is created with:

1. `WKWebsiteDataStore defaultDataStore`.
2. `mediaTypesRequiringUserActionForPlayback` set to none, so playback does not wait for a gesture.
3. The stored user agent applied before the first navigation.
4. A new-window request opens a second window and returns that `WKWebView`, so a login popup is not dropped. WebKit loads the request in the view that this method returns.

### 2.4 Menus

The application menu has About and Quit. About shows `PROJECT_NAME`, the version macros, and `PROJECT_DESCRIPTION`.

File, when bookmarks are enabled: AutoClick all bots, Bookmark, User Agent, Lua Script, Log, Exit.

File, when bookmarks are disabled: User Agent, Lua Script, Log, Exit. AutoClick and Bookmark are absent.

User Agent items, in order, using the macros in `project.h`: Webview (Default) `UA_WEBVIEW`, Google Chrome `UA_CHROME`, Mozilla Firefox `UA_FIREFOX`, Apple Safari `UA_SAFARI`, Microsoft Edge `UA_EDGE`. Choosing one saves `settings.sqlite`, updates the label, sets `customUserAgent`, and reloads.

AutoClick all bots does not navigate. The Linux bot list is only the empty sentinel, so the macOS item logs that no Follow click is scheduled.

Lua Script opens the userscripts directory. This build does not load sol2 and does not run `*.lua` files. The log line says that Lua execution stays on the Ubuntu program.

Log opens the log directory. The stored line is:

```text
[YYYY-MM-DD HH:MM:SS] pid:<pid> [LEVEL] @<file> :] <message>
```

`LEVEL` is `INFO`, `WARNING`, `ERROR`, or `DEBUG`. The console prints `WARN` for a warning. `DEBUG` lines are stored and printed only when `DEBUG` is set to a non-empty value other than `0`, or when config `[output] debug` is true. `--quiet` hides info, warning, and debug on the console. Errors still print. `--json` forces quiet. The stored file is still appended.

### 2.5 Command line

| Argument | Effect |
|----------|--------|
| `help`, `--help`, `-h` | Print usage and exit 0. No window |
| `--check` | Create the profile, open `settings.sqlite`, print `check passed`, exit 0. No window |
| `--shutdown=N` or `SHUTDOWN=N` | Close the window after N seconds when N is greater than 0 |
| `--bookmark=N` or `BOOKMARK=N` | See §2.3 |
| `--config PATH` | Read that file instead of the profile `config.conf` |
| `--quiet` or `QUIET=1` | Quiet console |
| `--json` or `JSON=1` | Quiet console |

A flag wins over the environment variable, and either one wins over `config.conf`.

### 2.6 What this macOS build does not do

These stay on the Linux program until a later requirement says otherwise:

- sol2 and Lua userscript execution
- the `log` and `luascript` command verbs
- the HTML log table inside the web view
- GStreamer environment variables
- compiling `src/main/cpp/*.cpp`
- writing `cookies.sqlite`
- a private WKWebView data folder under the profile

### 2.7 Why this requirement exists

- **CIAO Principle 5**: macOS behavior has one owner. Linux window law is not rewritten to pretend the GTK program is the macOS program.
- **CIAO Principle 1**: user-agent strings still have one definition, `project.h`.

## 3. Design principles

- **Caution**: the profile directories are created before the window opens. `--check` stops before `NSApplication` runs.
- **Intentional**: the binary name and `PROJECT_ID` are `gemini-player`. The Linux binary name stays `gemini-wilgat`.
- **Anti-fragile**: `--check` proves the profile without requiring the window to be visible over SSH.
- **Over-protect**: a new-window request is given a real popup view. It is not ignored. The requirement does not claim that `webkit/` holds the cookies.

## 4. Verification

| TP-ID | Pass |
|-------|------|
| TP-MAC-CHECK | `--check` exits 0, prints `check passed`, and leaves `settings.sqlite` under `$HOME/.app/gemini-player/`. The process does not open a window |
| TP-MAC-PLAYER | With `--shutdown=20` in a graphical session, the process starts, logs `WebKit view ready`, and exits after the timer. The startup URL is the start page |

Map: `docs/reviews/test-plan.md`. Both rows stay todo until they have been run on a Mac. On 2026-10-11 the macOS build host did not answer, so this file does not record a passing run.

## 5. Protection rule

Future editors must not copy user-agent string literals into `src/mac`, must not describe `cookies/` or `webkit/` as the WKWebView cookie store, and must not compile the GTK sources into the macOS `gemini-player`.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-cpp-macos-environment.md` | Toolchain |
| `docs/requirements/requirement-cpp-window.md` | Linux window |
| `src/mac/player_project.h` | macOS identity |
| `src/mac/mac_app.mm` | Window and WKWebView |
| `src/mac/mac_host.cpp` | Profile, config, settings |
| `src/mac/mac_log.cpp` | Stored log line |
| `src/main/cpp/project.h` | User-agent strings and version macros |

**Last updated**: 2026-10-11
**Owner**: macOS player
