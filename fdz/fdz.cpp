#include <Windows.h>
#include "BS_thread_pool.hpp"
#include <iostream>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <string>
#include "search_utils.h"
#include "Dir_utils.h"

#pragma comment(lib, "Shlwapi.lib")

void print_help(const char* program_name) {
    std::cout << R"(
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+  FDZ : Fast File Search                                     +
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

USAGE:
    )" << program_name << R"( [OPTIONS]
    )" << program_name << R"( [ROOT_PATH] <search_term>

OPTIONS:
    -h, --help          Show this help message
    -v, --version       Show version information

EXAMPLES:
    # Search entire system (user directories)
    )" << program_name << R"( myfile.txt

    # Search specific path
    )" << program_name << R"( C:\Projects myfile.txt

NOTES:
    skips: node_modules, cache, temp, recycle bin by default
)";
}

void print_version() {
    std::cout << "fdz version 0.0.1\n"
        << "Built with C++20\n"
        << "Fast file search tool\n"
        << "https://github.com/baiching/fdz\n";
}

int main(int argc, char* argv[]) {
    // Check for help/version FIRST (before any other logic)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_help("fdz");
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        }
    }

    std::string root_path;
    std::string target_file;
    Dir_Utils du;
    std::vector<std::string> current_batch = {};

    switch (argc) {
    case 1:
        // No arguments
        print_help("fdz");  // Show help instead of a terse message
        return 0;

    case 2:
        // Check if it's a directory or a search term
        if (std::filesystem::is_directory(argv[1])) {
            current_batch = { argv[1] };
            std::cerr << "Error: No search term provided.\n\n";
            print_help("fdz");
            return 1;
        }
        else {
            current_batch = du.get_user_search_dirs();
            target_file = argv[1];
        }
        break;

    case 3:
        current_batch = { argv[1] };
        target_file = argv[2];
        break;

    default:
        std::cerr << "Error: Too many arguments!\n\n";
        print_help("fdz");
        return 1;
    }

    std::cout << "Searching for: " << target_file << "\n";
    auto start = std::chrono::steady_clock::now();

    auto search = std::make_unique<Search_Utils>();
    auto matches = search->concurrent_search(current_batch, target_file);

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (matches.empty()) {
        std::cout << "No files found.\n";
    }
    else {
        std::cout << "Found " << matches.size() << " file(s) in " << elapsed << "ms:\n";
        for (const auto& path : matches)
            std::cout << "  " << path << "\n";
    }

    return 0;
}