# fdz

`fdz` is a file search tool that finds files *system wide* by default without requiring command-line flags. 
It doesn't aim to support the more powerful features offered by `fd` and `find`, 
but instead focuses on fast and straightforward file searching.

**Note** : Its windows only for now, as i don't have access to mac or linux distributions for the time being.

## Usage

It has three ways to search files

1. The simplest and the default search, it searches through user directories in C drive(`C:\users\<USERNAME>\Desktop`,
`C:\users\<USERNAME>\Downloads`, etc.) and all other drives except the full `C:\\` drive

```
fdz filename
```

2. Search at a specific location
```
fdz <path> filename
```
**Note** : If you provide only the path and no filenames, it will simply return everything on that directory

3. Search at current location
```
fdz . filename
```

**Examples:**
```
fdz invoice.pdf
fdz D:\ main.cpp
fdz C:\Users\<USERNAME>\Documents filename
fdz . tax                           <-- the dot means current location
```

## Build

### Prerequisites

- **Windows 10 SDK** or later
- **Visual Studio 2022** (or any MSVC toolchain supporting C++20)
- **CMake** 3.10+

### From Visual Studio

Open the `fdz/` folder in Visual Studio 2022, select configuration (Debug/Release) and build. CMake presets handle the rest.

### From terminal
[pending]

## Dependencies

All fetched automatically by CMake via `FetchContent`:

| Dependency | Use | Source |
|---|---|---|
| [BS::thread_pool](https://github.com/bshoshany/thread-pool) | Thread pool for parallel directory search | `BS_thread_pool.hpp` included in-tree |
| [Catch2](https://github.com/catchorg/Catch2) v3.15+ | Unit testing | FetchContent (auto) |
| [Moodycamel::Concurrentqueue](https://github.com/cameron314/concurrentqueue) | Lock-free queue | `concurrentqueue.h` included in tree |

## Testing

### From Visual Studio
On navigation bar select **Test** -> **run Ctests for fdz**

### From terminal
[pending]

## How it works (Brief Overview)
- Scans directories in parallel using a thread pool
- Batches directories to reduce contention
- Skips common noise folders (`node_modules`, `cache`, `temp` etc)

## License

[MIT](LICENSE.txt)
