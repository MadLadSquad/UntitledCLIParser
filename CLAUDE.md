# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

UntitledCLIParser (UCLI) is a C and C++ library for parsing CLI arguments, maintained by [MadLadSquad](https://madladsquad.com). It supports short/long arguments, grouped short switches (`-aBcD` == `-a -B -c -D`), variable assignment (`--test=value` or `--test value`), default arguments, nested subcommands, and array-typed arguments with terminator/probing semantics. It ships complete C and C++ APIs.

This repo is vendored as a git submodule inside UntitledImGuiFramework (`Framework/Modules/CLIParser/ThirdParty/`), but is a standalone library with its own CI, release, and packaging. End-user documentation lives in the [GitHub wiki](https://github.com/MadLadSquad/UntitledCLIParser/wiki) — consult it before guessing at usage conventions.

## Build & Test

There is no separate test suite; CI simply builds the library on Linux, Windows, and macOS. `ci.sh` is the canonical build:

```bash
bash ci.sh    # mkdir build, cmake .. -DCMAKE_BUILD_TYPE=RELEASE, then MSBuild (Windows) or make -j (Unix)
```

Manual build:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j$(nproc)
```

CMake options that change the output:
- `BUILD_VARIANT_STATIC` — build a `STATIC` library instead of the default `SHARED`.
- `UIMGUI_INSTALL` — enable `install()` rules (libs to `lib64/`, headers to `include/UntitledCLIParser/`, pkg-config `.pc` to `lib/pkgconfig/`).

Requires C++20 and C99. `UntitledCLIParser.pc.in` → `UntitledCLIParser.pc` is generated at configure time for pkg-config consumers.

## Architecture

The core is a single `UCLI::Parser` class (C++), wrapped 1:1 by an opaque-handle C API. A parse is a two-phase process: **build a callback queue**, then **execute it**.

### The parse pipeline (`Parser.cpp::parse`)

1. If auto-help is enabled, a `--help`/`help` command+flag is injected via `pushHelp()`.
2. Each `argv` token is classified: starts with `flagPrefix` (default `-`) → flag; otherwise → command. Bare `-` and `--` are reserved/skipped, not treated as flags.
3. **Commands** are matched against the current command's subcommand list (or the top-level list). On match, `currentCommand` descends into it and `_internal_parent` is set to walk back up.
4. **Flags** go through `Internal::parseFlag` → `findFlagsRecursive`, which searches the current command's flags and recurses *up* the parent chain (`_internal_parent`) so a flag declared on an ancestor command still resolves. Grouped shorts (`-fF`) iterate each character; the `bBatched` bit tells the value parser not to consume a following argument.
5. Every matched command/flag is handed to `executeCommand` (see routing below), which parses its value and appends a `CallbackObject` to the `callbacks` deque.
6. After the queue is built, flags are stable-sorted by `priority` (descending) within each command's flag run, then callbacks are invoked in order. A callback returning `UCLI_CALLBACK_RESULT_PREMATURE_EXIT` stops execution.

### Routing by type (`Router.hpp`)

`executeCommand<T>` is a template shared by both `Command` and `Flag` (they have compatible layouts). It dispatches on `command.type` to the type-specific value parser, then calls `pushCallback`:

- `UCLI_COMMAND_TYPE_BOOL`/`VOID` (both == 0) → `loadBooleanCommand` (`BooleanParser.hpp`)
- `UCLI_COMMAND_TYPE_STRING` → `loadStringCommand` (`StringParser.hpp`)
- `UCLI_COMMAND_TYPE_ARRAY` → `loadArrayCommand` (`ArrayParser.hpp`)

`pushCallback` is where ordering gets subtle: high-priority flags (`priority > SIZE_MAX/2`) go to the front of the queue; normal flags are inserted immediately after their owning command at the correct depth and the flag run is re-sorted by priority. This preserves `command → its flags` grouping even when flags were parsed before their command (the flag-probing case).

### Flag probing (`ArrayParser.hpp` + `FlagParser.cpp::probeFlags`)

The `git clone --recursive --depth 1 https://... folder` shape is handled specially: when an ARRAY-typed *command* is immediately followed by a flag, `probeFlags` parses flags forward until it hits a non-flag value, letting flags consume their own values and assigning the leftover value to the command. `bProbingFlags` on the parser tracks this state.

### Array parsing semantics

`ArrayParser.hpp` has a large header comment enumerating every accepted array syntax (`command=a,b,c`, `command a b c --`, `command a b --flag`, etc.). Two ownership modes matter for memory:
- **Assignment form** (`--flag=a,b,c`): UCLI `malloc`s and copies both the outer pointer array and each inner string → `_bFreeStringValues = true`, `_bFreeInnerStringValues = true`.
- **Sequence form** (`--flag a b c`): inner strings point directly into `argv` (owned by the toolchain), only the outer array is `malloc`d → `_bFreeStringValues = true`, `_bFreeInnerStringValues = false`.

### Memory ownership & cleanup (`CLIParser.cpp`)

`freeCommands`/`freeFlags` recursively walk subcommands/flags and free string/array values **only according to the `_internal_` ownership bits** set during parsing — never blindly. `useDefaultArguments`/`useNullArguments` (`ParserUtils.hpp`) always set both free-bits to `false` because those values are user-owned or null. `release()` frees all parsed values (destructor calls it); the parser can be reused after `release()`.

## C / C++ API relationship

- `Common.h` is the shared C ABI surface: the `UCLI_Command`/`UCLI_Flag` structs (note the `union` of `stringValues`/`boolValue` and the `_internal_` bookkeeping fields), enums, callback typedefs, and the `CLITERAL`/`UCLI_MAKE_*` inline-array macros. It is `extern "C"` and included by both the C++ headers and the C wrapper.
- The C++ side (`UCLI::Parser`) typedefs `Command`/`Flag`/`CommandType`/`CallbackResult` back to the `UCLI_*` C structs — they are the same types, not adapters.
- `C/cucli.h` + `C/cucli.cpp` are a thin wrapper: `UCLI_Parser` is an opaque `void*`, every `UCLI_Parser_*` function is a one-line `static_cast` to `UCLI::Parser*` forwarding to the matching method. When adding a public C++ method, add the matching `UCLI_Parser_*` C function and forwarder.
- `MLS_PUBLIC_API` (from `Common.h`) handles Windows `dllexport`/`dllimport`; it is empty on non-Windows. Exported symbols require `MLS_EXPORT_LIBRARY` + `MLS_LIB_COMPILE` (set as private compile defs in CMake).

## Conventions

- Public C++ setters return `Parser&` for fluent chaining and are `noexcept`. The whole library is `noexcept` throughout.
- Raw `malloc`/`free` (not `new`/`delete`) are used for all parsed value buffers, because they cross the C ABI boundary and their ownership is tracked by the `_internal_` struct bits. Use `UCLI_VOID_CAST` (`ParserUtils.hpp`) to strip const for stdlib calls.
- `defaultCommand`/`defaultFlag` are diagnostic sinks (e.g. `--help`, unknown-argument handlers). They are invoked *without* `executeCommand`, so they deliberately do not get default-value/array parsing; the offending token is passed via `_internal_ctx_`.
- Strict vs lenient mode: strict (default) runs the default command and exits on the first unknown token; lenient substitutes the default and keeps going.

## Releases & Automation

- Version lives in `CMakeLists.txt` `project(... VERSION x.y.z.0)`. The parent framework's "Automatically bump version" commits touch this.
- `.github/workflows/release.yaml` cuts releases; `update-dependencies.yaml` + `dependabot.yml` keep GitHub Actions current and open a daily dependency-update issue.
