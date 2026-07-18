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

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    // React only to Ctrl+C and (optionally) console‑close events.
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT)
    {
        // 1. Write the reset sequence directly to the console output.
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE)
        {
            DWORD written;
            WriteConsoleA(hOut, COLOR_RESET,
                static_cast<DWORD>(std::strlen(COLOR_RESET)),
                &written, NULL);
        }
        // 2. Terminate the process immediately – no further cleanup.
        ExitProcess(0);
    }
    // Let the default handler deal with any other event.
    return FALSE;
}


int wmain(int argc, wchar_t* argv[]) {
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
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
    std::setlocale(LC_ALL, "");

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

    switch (args.size()) {
    case 1:
        // No arguments
        current_batch = { "." };
        break;

    case 2:
        // Check if it's a directory or a search term
        if (std::filesystem::is_directory(args[1])) {
            current_batch = { args[1] };
            std::cerr << "Error: No search term provided.\n\n";
        }
        else {
            current_batch = du.get_user_search_dirs();
            target_file = args[1];
        }
        break;

    case 3:
        current_batch = { args[1] };
        target_file = args[2];
        break;

    default:
        std::cerr << "Error: Too many arguments!\n\n";
        print_help("fdz");
        return 1;
    }

    std::cout << "Searching for: " << target_file << "\n";
    auto start = std::chrono::steady_clock::now();

    search->concurrent_search(current_batch, target_file);

    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Found file(s) in " << elapsed << "ms:\n";

    return 0;
}
