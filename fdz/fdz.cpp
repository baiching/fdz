#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include "search_utils.h"
#include "dir_utils.h"
#include "util/utils.h"

int real_main(const std::vector<std::string>& args);

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::vector<std::string> args = convertall_wchar_to_utf8(argc, argv);

    real_main(args);

    return 0;

 }

#else
int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    args.reserve(argc);

    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    real_main(args);

    return 0;
}
#endif // _WIN32


int real_main(const std::vector<std::string> &args) {

    auto search = std::make_unique<Search_Utils>();


    // Check for help/version FIRST (before any other logic)
    for (int i = 1; i < args.size(); ++i) {
        std::string arg = args[i];

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
    search->concurrent_search(current_batch, target_file);

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    //if (matches.empty()) {
    //    std::cout << "No files found.\n";
    //}
    //else {
    //    std::cout << "Found " << matches.size() << " file(s) in " << elapsed << "ms:\n";
    //    for (const auto& path : matches)
    //        std::cout << "  " << path << "\n";
    //}

    std::cout << "Found file(s) in " << elapsed << "ms:\n";

    return 0;
}
