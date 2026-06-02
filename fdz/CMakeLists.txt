# CMakeList.txt : CMake project for fdz, include source and define
# project specific logic here.
#
include(FetchContent)
FetchContent_Declare(
  rapidfuzz
  GIT_REPOSITORY https://github.com/rapidfuzz/rapidfuzz-cpp.git
  GIT_TAG        v3.3.3 
)
FetchContent_MakeAvailable(rapidfuzz)

# Google benchmark
FetchContent_Declare(googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.5
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

add_library(dir_utils STATIC 
			"dir_utils.h" 
			"dir_utils.cpp"
 "search_utils.h" "search_utils.cpp")

# Add source to this project's executable.
add_executable (fdz "fdz.cpp" "fdz.h" "BS_thread_pool.hpp" "search_utils.h")

target_link_libraries(fdz PRIVATE rapidfuzz::rapidfuzz)
target_link_libraries(fdz PRIVATE benchmark::benchmark benchmark::benchmark_main)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET fdz PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests "tests/test_dir_utills.cpp" "tests/test_search_utils.cpp")
target_link_libraries(tests PRIVATE dir_utils Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tests)