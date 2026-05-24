# CMakeList.txt : CMake project for fdz, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (fdz "fdz.cpp" "fdz.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET fdz PROPERTY CXX_STANDARD 20)
endif()

# Find and link pthreads (via vcpkg on Windows, system pthread on Linux/macOS)
find_package(PThreads4W REQUIRED)
target_link_libraries(fdz PRIVATE PThreads4W::PThreads4W)

# TODO: Add tests and install targets if needed.
