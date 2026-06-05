#include <Windows.h>
#include "BS_thread_pool.hpp"
#include <atomic>
#include <iostream>
#include <optional>
#include <memory>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <filesystem>
#include <unordered_set>
#include <string>
#include "search_utils.h"
#include "Dir_utils.h"

#pragma comment(lib, "Shlwapi.lib")

int main(int argc, char* argv[])
{
    //std::string root_path =  std::filesystem::is_directory((std::filesystem::path)argv[1]) ? argv[1] : ".";
    std::string root_path;// = (argc > 1) ? argv[1] : "E:\\";
    std::string target_file;// = (argc > 2) ? argv[2] : "BaichingCV.pdf";
    Dir_Utils du;


    std::vector<std::string> current_batch = {};

    switch (argc) {
    case 1:
        // No arguments
        current_batch = du.get_user_search_dirs();  // or your default logic
        target_file = "baichingcv";
        std::cout << "Please enter arguments. EXAMPLE:\n" << "fdz [TARGET_PATH/DIRECTORY] [TARGET_PATH]" << std::endl;
        //return 0;
        break;

    case 2:
        // One argument - could be path or filename
        // You need business logic to decide
        if (std::filesystem::is_directory(argv[1])) {
            current_batch = { argv[1] };
            target_file = "";
        }
        else {
            current_batch = du.get_user_search_dirs();;  // current directory
            target_file = argv[1];
        }
        break;

    case 3:
        // Both arguments provided
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