# fdz — find fuzzy

A fast, multi-threaded Windows file finder that searches directories in parallel using Win32 API and BS::thread_pool.

No recursive directory walkers. No garbage abstractions. BFS + bulk-async — it discovers directories and searches them concurrently in a single tight loop.

## Usage

```
fdz <pattern>            Search user folders + non-C: drives
fdz <path> <pattern>     Search a specific directory
fdz <path>               Search a directory (interactive, enter pattern)
fdz                      No args — prints usage hint
```

**Examples:**
```
fdz invoice.pdf
fdz D:\projects main.cpp
fdz C:\Users\me\Documents tax
```

When no path is given and only one argument is provided, fdz automatically searches:
- Desktop, Downloads, Documents, Pictures, Music, Videos
- All fixed drives **except** C:\ (which might require special permissions to scan, to protect the system files)

Results are printed to stdout, one path per line.

## Build

### Prerequisites

- **Windows 10 SDK** or later
- **Visual Studio 2022** (or any MSVC toolchain supporting C++20)
- **CMake** 3.10+

### From command line

```powershell
cmake -B build
cmake --build build --config Release
.\build\fdz\Release\fdz.exe <pattern>
```

### From Visual Studio

Open the `fdz/` folder in Visual Studio 2022, select your configuration (Debug/Release), and build. CMake presets handle the rest.

## Dependencies

All fetched automatically by CMake via `FetchContent`:

| Dependency | Use | Source |
|---|---|---|
| [BS::thread_pool](https://github.com/bshoshany/thread-pool) | Thread pool for parallel directory search | `BS_thread_pool.hpp` included in-tree |
| [Catch2](https://github.com/catchorg/Catch2) v3.15+ | Unit testing | FetchContent (auto) |
| [Google Benchmark](https://github.com/google/benchmark) | Performance benchmarking | FetchContent (auto) |

No other external dependencies. No vcpkg, no Conan.

## Testing

```powershell
cmake -B build
cmake --build build --config Debug
cd build
ctest --output-on-failure
```

## How it works

- **BFS directory walk**: discovers directories level by level, not recursively. Avoids deep stacks and gives fine-grained parallelism.
- **Bulk-async dispatch**: batches of 64 directories are detached into the thread pool; the main thread concurrently harvests the next level of subdirectories while the pool processes the current level.
- **Win32 `FindFirstFileExW`** with `FIND_FIRST_EX_LARGE_FETCH` for fast directory enumeration.
- **Case-insensitive matching** via exact match and substring match on lowered filenames.
- **Skip list**: ignores VCS dirs (`.git`), build output (`Debug`, `build`, `_deps`, `node_modules`), IDE junk (`.vs`, `.idea`), and other noise — see `dir_utils.cpp` for the full list.

## Project structure

```
fdz/
├── CMakeLists.txt              Top-level (C++20, MSVC config)
└── fdz/
    ├── CMakeLists.txt          Sources, tests, FetchContent deps
    ├── fdz.cpp / .h            CLI entry point
    ├── search_utils.cpp / .h   File search + concurrent BFS engine
    ├── dir_utils.cpp / .h      Directory utilities, skip list, drive detection
    ├── BS_thread_pool.hpp      BS::thread_pool (header-only, v5.1.0)
    └── tests/
        ├── test_dir_utills.cpp
        └── test_search_utils.cpp
```

## License

MIT
