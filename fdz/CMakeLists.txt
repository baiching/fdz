# CMakeList.txt : CMake project for fdz, include source and define
# project specific logic here.
#
include(FetchContent)


# adding helper files
add_library(conqueue INTERFACE)
target_include_directories(conqueue INTERFACE ${PROJECT_SOURCE_DIR})

# Add source to this project's executable.
add_executable (fdz "fdz.cpp" "fdz.h" "search_utils.h" "search_utils.cpp" "dir_utils.h" 
			"dir_utils.cpp" "BS_thread_pool.hpp" "task_queue.h" "task_queue.cpp" "libs/logger.cpp")

#target_link_libraries(fdz PRIVATE conqueue)
#target_link_libraries(fdz PRIVATE benchmark::benchmark benchmark::benchmark_main)


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
    "search_utils.cpp"
	"dir_utils.cpp" "task_queue.h" "task_queue.cpp" "libs/logger.cpp")

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tests)

if(MSVC)
    target_compile_options(fdz PRIVATE 
		/arch:AVX2 
        /fp:fast
        /O2
	)
else()
    target_compile_options(fdz PRIVATE 
		-mavx2 
        -mfma 
        -O3
	)
endif()