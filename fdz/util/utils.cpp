#include "utils.h"
#include <iostream>

std::string wchar_to_utf8(const wchar_t* wstr) {
    int len = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr);

    if (len <= 0) return {};

    std::string str(len - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        &str[0],
        len,
        nullptr, nullptr);

    return str;
}

std::wstring utf8_to_wchar(const char* utf8_str) {
    int len = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8_str,
        -1,
        nullptr,
        0);

    if (len <= 0) return {};

    std::wstring wstr(len - 1, L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8_str,
        -1,
        &wstr[0],
        len);

    return wstr;
}

std::vector<std::string> convertall_wchar_to_utf8(int argc, wchar_t* argv[]) {
    std::vector<std::string> args;
    args.reserve(argc);

    for (size_t i = 0; i < argc; i++)
    {
        args.push_back(wchar_to_utf8(argv[i]));
    }

    return args;
}

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