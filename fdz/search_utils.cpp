#include "search_utils.h"
#include "dir_utils.h"
#include <algorithm>
#include <Windows.h>
#include <chrono>
#include <memory>
#include "task_queue.h"
#include "libs/logger.h"
#include <iostream>
#include <syncstream>
#include <filesystem>
#include "util/utils.h"

moodycamel::ConcurrentQueue<std::string> result_q;
namespace fs = std::filesystem;

void Search_Utils::find_file(const std::string& path,
	const std::string& target,
	const std::atomic<bool>* stop,
    std::vector<std::string> &dirs,
    Dir_Utils &dir_utils,
    std::unordered_set<std::string> &skip_list)
{
    std::string color;
    std::wstring query = utf8_to_wchar(target.c_str());
    std::wstring pattern = utf8_to_wchar(path.c_str()) + L"\\*";

    WIN32_FIND_DATAW ffd;
    dir_utils.wlower(query);

    HANDLE hFind = FindFirstFileExW(
        pattern.c_str(),
        FindExInfoBasic,
        &ffd,
        FindExSearchNameMatch,
        nullptr,
        FIND_FIRST_EX_LARGE_FETCH
    );

    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (stop && *stop) break;

        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0) continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { 
            //modified
            std::string filename = wchar_to_utf8(ffd.cFileName);
            std::string fname = filename;
            color = COLOR_DIR;

            for (char& c : fname) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            if (skip_list.find(fname) == skip_list.end())
            {
                dirs.push_back(path + "\\" + filename);
            }
        }

        else if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            color = COLOR_REPARSE;
        }
        else if(ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        {
            color = COLOR_HIDDEN;
        }
        else
        {
            color = COLOR_FILE;
        }

        std::wstring fname(ffd.cFileName);
        std::wstring fname_lower = fname;
        dir_utils.wlower(fname_lower);

        bool matched = false;

        if (fname_lower == query) {
            matched = true;
        }

        else if (fname_lower.find(query) != std::wstring::npos) {
            matched = true;
        }

        if (matched) {
            //result_q.enqueue(path + "\\" + wchar_to_utf8(ffd.cFileName));
            std::osyncstream(std::cout) << COLOR_DIR << path << "\\" << color << wchar_to_utf8(ffd.cFileName) << COLOR_RESET <<'\n';
        }

    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
}

void Search_Utils::process_directories(std::unique_ptr<batch_s> batch,std::string &target, Search_Utils &srh, size_t batch_size) {
    Dir_Utils dirutils;
    std::vector<std::string> dir_collection;
    dir_collection.reserve(1024);
    std::unordered_set<std::string> skip_list = dirutils.get_skip_list();
    //std::unique_ptr<batch_s> batch = get_batch();

    for (const auto &dir : batch->data) {
        srh.find_file(dir, target, nullptr, dir_collection, dirutils, skip_list);
    }


    for (size_t i = 0; i < dir_collection.size(); i+=batch_size)
    {
        std::size_t end = std::min<std::size_t>(i + batch_size, dir_collection.size());

        auto new_batch = std::make_unique<batch_s>();
        new_batch->data.reserve(batch_size);
        new_batch->data.assign(
            dir_collection.begin() + i,
            dir_collection.begin() + end
        );

        add_batch(std::move(new_batch));
    }
}

void Search_Utils::concurrent_search(
    const std::vector<std::string>& roots,
    std::string& target
) {
    BS::thread_pool pool;
    std::vector<std::string> results;
    Search_Utils srh;
    Dir_Utils dir_utils;
    size_t batch_size = 256;

    std::cout << "results : " << std::endl;

    auto initial_batch = std::make_unique<batch_s>();
    initial_batch->data = roots;
    add_batch(std::move(initial_batch));

    std::atomic<size_t> tasks_running{ 0 };

    while (true)
    {
        std::unique_ptr<batch_s> batch = get_batch();

        if (batch)
        {
            ++tasks_running;

            pool.detach_task([
                batch = std::move(batch),
                &target,
                &srh,
                &tasks_running,
                batch_size
            ]() mutable {
                    srh.process_directories(std::move(batch), target, srh, batch_size);
                    --tasks_running;
                });
            
        }
        else {
            if (tasks_running.load() == 0)
            {
                break;
            }
            std::this_thread::yield();
        }
    }
    std::cout << std::endl;
    pool.wait();

    //std::string item;
    //while (result_q.try_dequeue(item)) {
    //    results.push_back(std::move(item));
    //}

    //return results;
}