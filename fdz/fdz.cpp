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

int main(int argc, char* argv[])
{
    std::string root_path;
    std::string target_file;
    Dir_Utils du;


    std::vector<std::string> current_batch = {};

    switch (argc) {
    case 1:
        // No arguments
        current_batch = du.get_user_search_dirs();  
        target_file = "";
        std::cout << "Please enter arguments. EXAMPLE:\n" << "fdz [TARGET_PATH/DIRECTORY] [TARGET_PATH]" << std::endl;
        return 0;
        break;

    case 2:
        if (std::filesystem::is_directory(argv[1])) {
            current_batch = { argv[1] };
            target_file = "";
            std::cout << "Please enter arguments. EXAMPLE:\n" << "fdz [TARGET_PATH/DIRECTORY] [TARGET_PATH]" << std::endl;
            return 0;
        }
        else {
            current_batch = du.get_user_search_dirs();;  
            target_file = argv[1];
        }
        break;

    case 3:
        current_batch = { argv[1] };
        target_file = argv[2];
        break;

    default:
        std::cerr << "Too many arguments!" << std::endl;
        return 1;
    }

    std::atomic<bool> found = false;
    std::string result_path;
    std::mutex result_mutex;

    auto search = std::make_unique<Search_Utils>();
    auto matches = search->concurrent_search(current_batch, target_file);

    if (matches.empty()) {   
        std::cout << "No files found.\n";
    }
    else {
        std::cout << "Found " << matches.size() << " file(s):\n";
        for (const auto& path : matches)
            std::cout << path << "\n";
    }
}