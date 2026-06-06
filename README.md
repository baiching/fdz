# fdz: find fuzzy

A fast, multi-threaded Windows file finder that searches directories in parallel using Win32 API and BS::thread_pool.

BFS style walk + batch directories processing. It discovers directories and searches them concurrently in a single tight loop.

## Usage

```
fdz <pattern>            Search user folders + non-C: drives
fdz <path> <pattern>     Search a specific directory
fdz                      No args: prints usage hint
```

**Examples:**
```
fdz invoice.pdf
fdz D:\projects main.cpp
fdz C:\Users\me\Documents tax
fdz . tax                           <-- the dot means current location
```

When no path is given and only one argument is provided, fdz automatically searches:
- Desktop, Downloads, Documents, Pictures, Music, Videos
- All fixed drives **except** C:\ (which might require special permissions to scan, to protect the system files)
- By Default it skips C:\ drive, but if explicily givent C-drive as the path, it will scan that drive

Results are printed to stdout, one path per line.

## Build

### Prerequisites

- **Windows 10 SDK** or later
- **Visual Studio 2022** (or any MSVC toolchain supporting C++20)
- **CMake** 3.10+

### From Visual Studio

Open the `fdz/` folder in Visual Studio 2022, select your configuration (Debug/Release) and build. CMake presets handle the rest.

## Dependencies

All fetched automatically by CMake via `FetchContent`:

| Dependency | Use | Source |
|---|---|---|
| [BS::thread_pool](https://github.com/bshoshany/thread-pool) | Thread pool for parallel directory search | `BS_thread_pool.hpp` included in-tree |
| [Catch2](https://github.com/catchorg/Catch2) v3.15+ | Unit testing | FetchContent (auto) |
| [Google Benchmark](https://github.com/google/benchmark) | Performance benchmarking | FetchContent (auto) |

## Testing

### From Visual Studio
On navigation bar select **Test** -> **run Ctests for fdz**

## How it works
- Scans directories in parallel using a thread pool
- Batches directories to reduce contention
- Skips common noise folders (`node_modules`, `cache`, `temp`)
- No indexing, no background service – just fast traversal

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

[MIT](LICENSE.txt)
