# CMakeList.txt : CMake project for fdz, include source and define
# project specific logic here.
#
include(FetchContent)

# Google benchmark
FetchContent_Declare(googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.5
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_MakeAvailable(googlebenchmark)

# adding helper files
#add_library(dir_utils STATIC 
#			"dir_utils.h" 
#			"dir_utils.cpp"
#"BS_thread_pool.hpp")
#junk, not needed
#target_include_directories(dir_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Add source to this project's executable.
add_executable (fdz "fdz.cpp" "fdz.h" "search_utils.h" "search_utils.cpp" "dir_utils.h" 
			"dir_utils.cpp" "BS_thread_pool.hpp")

#target_link_libraries(fdz PRIVATE dir_utils)
target_link_libraries(fdz PRIVATE benchmark::benchmark benchmark::benchmark_main)


# TODO: Tests and install targets.
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests 
	"tests/test_dir_utills.cpp" 
	"tests/test_search_utils.cpp"
	"search_utils.h"
    "search_utils.cpp"
	"dir_utils.h" 
	"dir_utils.cpp")
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tests)