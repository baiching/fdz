#pragma once
#ifdef _WIN32
#include<Windows.h>
#endif // _WIN32

#include<string>
#include <vector>

// it has gathered the helper function of different types

std::vector<std::string> convertall_wchar_to_utf8(int argc, wchar_t* argv[]);
std::string wchar_to_utf8(const wchar_t* wstr);
std::wstring utf8_to_wchar(const char* utf8_str);
void print_help(const char* program_name);
void print_version();
