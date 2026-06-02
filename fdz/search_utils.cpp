#include "search_utils.h"
#include <Windows.h>

std::vector<std::string> Search_Utils::find_file(const std::string& path,
	const std::string& target,
	const std::atomic<bool>* stop)
{
	// Implementation moved to fdz.cpp for simplicity
	return {};
}

std::vector<std::string> Search_Utils::list_subdirs(const std::string& path,
	const std::atomic<bool>* stop) {
    std::vector<std::string> dirs;
    std::wstring pattern = std::wstring(path.begin(), path.end()) + L"\\*";
    WIN32_FIND_DATAW ffd;

    HANDLE hfind = FindFirstFileExW(
        pattern.c_str(),
        FindExInfoStandard,
        &ffd,
        FindExSearchNameMatch,
        nullptr,
        FIND_FIRST_EX_LARGE_FETCH
    );

    if (hfind == INVALID_HANDLE_VALUE) return dirs;

    do {
        if (stop && *stop) break;

        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0) continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            int len = WideCharToMultiByte(CP_UTF8, 0, ffd.cFileName, -1, nullptr, 0, nullptr, nullptr);
            std::string name(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, ffd.cFileName, -1, &name[0], len, nullptr, nullptr);
            dirs.push_back(path + "\\" + name);
        }

    } while (FindNextFileW(hfind, &ffd) != 0);

    FindClose(hfind);
    return dirs;
}