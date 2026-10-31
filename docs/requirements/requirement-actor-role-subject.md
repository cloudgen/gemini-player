**file**: docs/requirements/requirement-actor-role-subject.md
**Status**: Active
**Updated**: 2026-10-31
**Philosophy**: CIAO v2.10.2 / CIAO-Lite (Caution • Intentional • Anti-fragile • Over-engineered / Over-protect)

## 1. Purpose

This file owns who runs gemini-wilgat and the Windows and macOS player. The table has three columns: Actor, Role, and Subject. There is no approval desk, so there is no Approver column and no Submitter column.

### 1.1 Human-facing

**In one sentence:** You are the person at this login, each job you run is a role, and Subject stays None because nobody submits a request for someone else to approve.

| Box | Meaning | Example |
|-----|---------|---------|
| You / this login | The person who starts the program or the build | You run `gemini-wilgat help` |
| The other role | The job named in the table | help, log, luascript, logcheck, install, or build |
| Not this file | The menu items, the log actions, and the window | `requirement-cpp-ui-menu.md`, `requirement-cpp-log-commands.md`, `requirement-cpp-window.md` |

| Includes | Excludes |
|----------|----------|
| One row for each verb family people run, including help and install | An Approver column, a Submitter column, and a second actor file |

| Surface | What you open | What for |
|---------|---------------|----------|
| The terminal | `help`, `log`, `luascript`, `logcheck` | Those jobs stop before the window |
| The window | File and About | The same person, after the window is open |
| `./build.sh` | `install`, `deps`, `clean`, `build`, `simple` | The developer build on Ubuntu |

| You do… | What it means | What you type |
|---------|---------------|---------------|
| Read help | Usage text, then stop | `gemini-wilgat help` |
| Install host packages | Bootstrap only. The window does not call sudo | `./build.sh install` |
| Build | Compile the tree for this host | `./build.sh build` |

## 2. Core rules

1. The catalog prints Actor, Role, and Subject. It does not print Submitter or Approver.
2. Every verb family people run has at least one row.
3. Help and install have no dest request. Their Subject is None.
4. When a dest approval desk exists, this three-column table is no longer the catalog. This product has no approval desk.

### 2.1 Implementation notes (this project)

Actor is this login. Subject is None on every row.

| Actor | Role | Subject |
|-------|------|---------|
| this login | starts the window and uses File and About | None |
| this login | reads `help` | None |
| this login | runs `log` (`list`, `show`, `folder`, `clear`) | None |
| this login | runs `luascript` (`folder`, `list`, `view`, `edit`, `new`) | None |
| this login | runs `logcheck` | None |
| this login | runs `./build.sh install` or `deps` | None |
| this login | builds or cleans (`./build.sh clean`, `build`, `simple`; Windows `build-win.bat`; macOS `build-mac.zsh`) | None |

`log` is `requirement-cpp-log-commands.md`. `luascript` is `requirement-cpp-lua-script-commands.md`. `logcheck` is `requirement-cpp-log-format.md`. Help lists those verbs in `requirement-cpp-parameters.md`. `./build.sh` verbs are `requirement-cpp-skills-installation-basic.md`. Ant and Gradle verbs stay Deferred and are not rows here.

## 3. Design principles

- **Caution**: an empty Actor cell is not a row. Subject None is written, not left blank.
- **Intentional**: the class file points here. The class sentence does not replace this table.
- **Anti-fragile**: a new verb family adds a row in the same change as the verb.
- **Over-protect**: no Approver column is added to make the sheet look complete.

## 4. Verification

| TP-ID | What a pass looks like | Status |
|-------|------------------------|--------|
| TP-ARS-TABLE | This file's table has the columns Actor, Role, and Subject only. Help and install are rows. Subject on those rows is None. The words Approver and Submitter are not column names | todo |

`docs/reviews/test-plan.md` keeps that status. Do not mark the row done until an executable check has been run.

## 5. Protection rule

Future editors must not add an Approver column, add a Submitter column, invent an approval account, publish a second actor file, or drop the help row or the install row.

## 6. Related artifacts

| Artifact | Role |
|----------|------|
| `docs/requirements/index.md` | Registry |
| `docs/requirements/requirement-class-software-dev.md` | Points here. No approval desk |
| `docs/requirements/requirement-cpp-parameters.md` | `help`, `log`, `luascript`, `logcheck` |
| `docs/requirements/requirement-cpp-skills-installation-basic.md` | `install`, `deps`, `clean`, `build`, `simple` |
| `docs/requirements/requirement-cpp-ui-menu.md` | File and About |
| `docs/requirements/requirement-cpp-window.md` | The live window |
| `docs/requirements/requirement-cpp-log-commands.md` | `log` |
| `docs/requirements/requirement-cpp-lua-script-commands.md` | `luascript` |
| `docs/requirements/requirement-cpp-log-format.md` | `logcheck` |
| `docs/requirements/requirement-cpp-tests.md` | `TP-ARS-TABLE` |
| `docs/reviews/test-plan.md` | Status of `TP-ARS-TABLE` |

## Terminologies

**Actor / role / subject** is the light consider catalog for a software-development program that has no dest approver: who runs a verb, in what role, and for whom, if anyone. The printed table has three columns: Actor, Role, and Subject. It must not grow an Approver column.

**Human daily-life explanation.** Actor / role / subject is the short who-is-who list for a kitchen that has no stamp person: who cooks, in what job, and whose cake it is, if anyone's. You still write the list. You do not add a fake teacher signature.

**Daily-life example.** A home bake has a mixer (actor), the job "mix" (role), and None for whose form it is (subject) because nobody submits that bake for a stamp. You do not invent a vice-principal so the sheet looks official.

## 7. Status history

- **1.0.0** (2026-10-31): Light catalog. Columns Actor, Role, Subject. No Approver column.

**Last updated**: 2026-10-31
**Owner**: Actor
