## fdz v1.0.0

Previous version was inefficient at task distribution and was doing io operations twice for each directory. Once to list directories
then it reads again to match the files. So this version addresses that issue.

### What's new
- One I/O for get directory lists and find file matches
- Added lock-free `moodycamel::concurrentqueue.h` library to ensure task distributions happens efficiently
- Added some color combinations for results
- UTF support
- Processing data in bigger batches
- Returns a basic result even if there is no file names are provided