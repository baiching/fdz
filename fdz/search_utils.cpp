#include "search_utils.h"
#include "dir_utils.h"
#include <algorithm>
#include <Windows.h>
#include <chrono>

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

static std::wstring utf8_to_wchar(const char* utf8_str) {
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

static int compute_max_dist(size_t len) {
    if (len == 0) return 0;
    int limit = static_cast<int>(len / 3);
    if (limit < 1) limit = 1;
    if (limit > 2) limit = 2;
    return limit;
}

std::vector<std::string> Search_Utils::find_file(const std::string& path,
	const std::string& target,
	const std::atomic<bool>* stop,
    Dir_Utils &dir_utils)
{
	//Dir_Utils dir_utils;
    std::vector<std::string> result;
    result.reserve(1024);
	
    std::wstring query = utf8_to_wchar(target.c_str());
    std::wstring pattern = utf8_to_wchar(path.c_str()) + L"\\*";

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

        std::wstring fname(ffd.cFileName);
        std::wstring fname_lower = fname;
        dir_utils.lower(fname_lower);

        bool matched = false;

        if (fname_lower == query) {
            matched = true;
        }

        else if (fname_lower.find(query) != std::wstring::npos) {
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

static void process_batch(
    std::vector<std::string>& batch, 
    std::string &target,
    std::vector<std::string> &out_results,
    std::mutex &result_mutex,
    Search_Utils &srh,
    Dir_Utils &dir_utils
) {
    for (const auto &dir : batch) {
        auto local = srh.find_file(dir, target, nullptr, dir_utils);

        if (!local.empty()) {
            std::lock_guard lock(result_mutex);
            out_results.insert(
                out_results.end(),
                std::move_iterator(local.begin()),
                std::move_iterator(local.end())
            );
        }
    }
}

static std::vector<std::string> gather_bulk(
    std::vector<std::string> &batch,
    Search_Utils &srh,
    Dir_Utils &dir_utils
    ) {
    std::vector<std::string> dir_bulk;

    std::unordered_set<std::string> skip_list = dir_utils.get_skip_list();
    for (const auto &dir : batch) {
        auto subdirs = srh.list_subdirs(dir, NULL);

        for (const auto& sub : subdirs) {
            std::string fname = dir_utils.filename_from_path(sub);
            for (char& c : fname)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (skip_list.find(fname) == skip_list.end()) {
                dir_bulk.push_back(sub);
            }
        }
    }

    return dir_bulk;
}

void split_and_submit(
    std::vector<std::string> &bulk,
    std::string &target,
    std::vector<std::string> &out_results,
    std::size_t batch_size,
    BS::thread_pool<> &pool,
    Search_Utils &srh,
    Dir_Utils &dir_utils
) {

    auto result_mutex = std::make_shared<std::mutex>();

    if (bulk.empty()) return;

    for (size_t i = 0; i < bulk.size(); i += batch_size) {
        std::size_t end = std::min<std::size_t>(i + batch_size, bulk.size());

        std::vector<std::string> batch(bulk.begin() + i, bulk.begin() + end);

        pool.detach_task([
            batch = std::move(batch),
            &target,
            &out_results,
            result_mutex,
            &srh,
            &dir_utils
        ] () mutable {
            process_batch(batch, target, out_results, *result_mutex, srh, dir_utils);
            });
    }
}

std::vector<std::string> Search_Utils::concurrent_search(
    const std::vector<std::string>& roots,
    std::string& target
) {
    BS::thread_pool pool;
    std::vector<std::string> results;
    Search_Utils srh;
    Dir_Utils dir_utils;

    std::vector<std::string> current_data = roots;
    //size_t file_counter = 0;
    std::cout << "searching batch : ";
    while (!current_data.empty())
    {
        split_and_submit(current_data, target, results, 128, pool, srh, dir_utils);
        current_data = gather_bulk(current_data, srh, dir_utils);
        std::cout << ".";
    }
    std::cout << std::endl;
    pool.wait();

    return results;
}