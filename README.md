# fdz

*fdz* is a fast file search tool for Windows. 
By default, it performs a system-wide search without requiring additional command-line flags.
It doesn't aim to support the more powerful features offered by `fd` and `find`, 
but instead focuses on fast and straightforward file searching.

**Note** : Currently Windows only. 
Linux and macOS support are planned, but I don't currently have access to those platforms for development.

## Why not use this?

- If you prefer a GUI, this is probably not for you. However, if you ever build one yourself, fdz could work well as a fast backend search component.
- If you prefer fine-grained control through flags and explicit search configuration, traditional tools are likely a better fit.

## Usage

It has three ways to search files

1. The simplest and the default search, it searches through user directories in C drive(`C:\users\<USERNAME>\Desktop`,
`C:\users\<USERNAME>\Downloads`, etc.) and every other mounted drive, while intentionally skipping a full scan of C:\ for faster searches.

```bash
fdz filename
```

2. Search at a specific location
```bash
fdz <path> filename
```
**Note** : If you provide only the path and no filenames, it will simply return everything on that directory

3. Search at current location
```bash
fdz . filename
```

**Examples:**
```bash
fdz filename.pdf
fdz D:\ main.cpp
fdz C:\Users\<USERNAME>\Documents filename
fdz . filename                           <-- the dot means current location
```

### System wide default benchmark
This section demonstrates the performance of fdz in its default system-wide search mode.
```bash
Benchmark 1: fdz .dll
  Time (mean ± σ):     396.2 ms ±  25.0 ms    [User: 429.1 ms, System: 1411.9 ms]
  Range (min … max):   369.3 ms … 438.3 ms    10 runs
```

## Benchmarks
I have compared `fdz` against [fd](https://github.com/sharkdp/fd)(v10.4.2) to get a proper comparison. I am using [hyperfine](https://github.com/sharkdp/hyperfine), it is a brilliant tool from the same author as `fd`.
I have chose `C:\` drive to perform the searches as its the biggest and complex place i have to get a good understanding:

**case 1**: searching all the `dll` files in C drive  
Query: 
```bash
hyperfine --warmup 3 "fd .dll C:\\" "fdz C:\\ .dll"  
Benchmark 1: fd .dll C:\\  
  Time (mean ± σ):     14.656 s ±  0.090 s    [User: 13.071 s, System: 72.557 s]  
  Range (min … max):   14.503 s … 14.786 s    10 runs  
    
Benchmark 2: fdz C:\\ .dll  
  Time (mean ± σ):      5.410 s ±  0.158 s    [User: 6.053 s, System: 22.593 s]  
  Range (min … max):    5.152 s …  5.616 s    10 runs  
```

Result:
```bash
fdz C:\\ .dll ran
    2.71 ± 0.08 times faster than fd .dll C:\`\
```

**case 2**: searching dll files in WinSxS directory which is messy by nature  

Query : 
```bash
hyperfine --warmup 3 "fd .dll C:\\Windows\\\WinSxS" "fdz C:\\Windows\\WinSxS .dll"  
Benchmark 1: fd .dll C:\\Windows\\\WinSxS  
  Time (mean ± σ):     14.383 s ±  0.974 s    [User: 13.665 s, System: 65.596 s]  
  Range (min … max):   13.476 s … 16.378 s    10 runs  

Benchmark 2: fdz C:\\Windows\\WinSxS .dll  
  Time (mean ± σ):      2.195 s ±  0.197 s    [User: 2.451 s, System: 7.201 s]  
  Range (min … max):    1.986 s …  2.550 s    10 runs  
  ```
Results:
```bash
  fdz C:\\Windows\\WinSxS .dll ran
    6.55 ± 0.74 times faster than fd .dll C:\\Windows\\\WinSxS
 ```

## Benchmark Environment

All benchmarks were run on the following system:

- **OS:** Windows 11 Pro
- **CPU:** Intel Core i7-1165G7 (11th Gen, 4 cores / 8 threads, 2.80 GHz)
- **RAM:** 16 GB
- **Storage:** NVMe SSD
- **System Drive:** 140 GB (`C:`)

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
| [GoogleTest](https://github.com/google/googletest) | Unit testing | FetchContent (auto) |
| [Moodycamel::Concurrentqueue](https://github.com/cameron314/concurrentqueue) | Lock-free queue | `concurrentqueue.h` included in tree |

## Testing

I am using google test to the functions.

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
