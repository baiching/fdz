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

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
# Add source to this project's executable.
add_executable (fdz "fdz.cpp" "fdz.h" "BS_thread_pool.hpp")

target_link_libraries(fdz PRIVATE rapidfuzz::rapidfuzz)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET fdz PROPERTY CXX_STANDARD 20)
endif()

# Find and link pthreads (via vcpkg on Windows, system pthread on Linux/macOS)

# TODO: Add tests and install targets if needed.
