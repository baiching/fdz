#include "search_utils.h"
#include "dir_utils.h"
#include <algorithm>
#include <Windows.h>

static std::string wchar_to_utf8(const wchar_t* wstr) {
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

std::vector<std::string> Search_Utils::find_file(const std::string& path,
	const std::string& target,
	const std::atomic<bool>* stop)
{
	Dir_Utils dir_utils;
    std::vector<std::string> result;
    std::string query = target;
    std::wstring pattern = std::wstring(path.begin(), path.end()) + L"\\*";
    WIN32_FIND_DATAW ffd;
    dir_utils.lower(query);

    HANDLE hFind = FindFirstFileExW(
        pattern.c_str(),
        FindExInfoBasic,
        &ffd,
        FindExSearchNameMatch,
        nullptr,
        FIND_FIRST_EX_LARGE_FETCH
    );

    if (hFind == INVALID_HANDLE_VALUE) return result;

    do {
        if (stop && *stop) break;

        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0) continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        std::string fname(wchar_to_utf8(ffd.cFileName));
        std::string fname_lower = fname;
        dir_utils.lower(fname_lower);

        bool matched = false;

        if (fname_lower == query) {
            matched = true;
        }

        else if (fname_lower.find(query) != std::string::npos) {
            matched = true;
        }

        else {
            size_t dist = rapidfuzz::levenshtein_distance(query, fname_lower);
            int max_dist = 0;
            if (query.empty()) {
                max_dist = 0;
            }
            else {
				size_t dynamic_limit = query.size() / 3;

				size_t edit_distance_limit = (dynamic_limit < 2) ? dynamic_limit : 2;

				if (edit_distance_limit == 0) edit_distance_limit = 1;
                else
                {
					max_dist = static_cast<int>(edit_distance_limit);
                }
            }
            if (max_dist < 1) max_dist = 1;
            if (dist <= (size_t)max_dist)
                matched = true;
        }

        if (matched) {
            result.push_back(path + "\\" + wchar_to_utf8(ffd.cFileName));
        }

    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
    return result;
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
        FindExSearchLimitToDirectories,
        nullptr,
        FIND_FIRST_EX_LARGE_FETCH
    );

    if (hfind == INVALID_HANDLE_VALUE) return dirs;

    do {
        if (stop && *stop) break;

        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0) continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dirs.push_back(path + "\\" + wchar_to_utf8(ffd.cFileName));
        }

    } while (FindNextFileW(hfind, &ffd) != 0);

    FindClose(hfind);
    return dirs;
}