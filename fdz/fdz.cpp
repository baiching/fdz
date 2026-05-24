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

#pragma comment(lib, "Shlwapi.lib")

std::vector<std::string> get_fixed_drives() {
    std::vector<std::string> drives;
    char buf[512];
    DWORD len = GetLogicalDriveStringsA(sizeof(buf), buf);
    if (len == 0) return drives;
    char* p = buf;
    while (*p) {
        std::string root = p;                         // e.g., "C:\"
        if (GetDriveTypeA(root.c_str()) == DRIVE_FIXED) // only physical HDD/SSD
            drives.push_back(root);
        p += root.size() + 1;                         // skip null terminator
    }
    return drives;
}

std::string known_folder_path(const KNOWNFOLDERID& id) {
    PWSTR wpath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, NULL, &wpath))) {
        // Convert wide to ANSI
        int len = WideCharToMultiByte(CP_ACP, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
        std::string path(len - 1, '\0');
        WideCharToMultiByte(CP_ACP, 0, wpath, -1, &path[0], len, nullptr, nullptr);
        CoTaskMemFree(wpath);
        return path;
    }
    return {};
}

std::vector<std::string> get_user_search_dirs() {
    std::vector<std::string> dirs;
    auto add = [&](const std::string& d) {
        if (!d.empty() && GetFileAttributesA(d.c_str()) != INVALID_FILE_ATTRIBUTES)
            dirs.push_back(d);
        };
    add(known_folder_path(FOLDERID_Desktop));
    add(known_folder_path(FOLDERID_Downloads));
    add(known_folder_path(FOLDERID_Documents));
    add(known_folder_path(FOLDERID_Pictures));
    add(known_folder_path(FOLDERID_Music));
    add(known_folder_path(FOLDERID_Videos));
    auto rest = get_fixed_drives();
    std::erase(rest, "C:\\");
    dirs.insert(dirs.end(), rest.begin(), rest.end());
    return dirs;
}

std::optional<std::string> find_file(const std::string& path,
    const std::string& target,
    const std::atomic<bool>* stop = nullptr)
{
    std::string path_pattern = path + "\\" + target;             
    WIN32_FIND_DATAA find_info;

    HANDLE hfind = FindFirstFileA(path_pattern.c_str(), &find_info);
    if (hfind == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }

    std::optional<std::string> result;
    do {
        if (stop && *stop) break;

        if (_stricmp(find_info.cFileName, ".") == 0 || _stricmp(find_info.cFileName, "..") == 0) continue;

        if (find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        if (PathMatchSpecA(find_info.cFileName, target.c_str())) {
            result = path + "\\" + find_info.cFileName;
            break;
        }
    } while (FindNextFileA(hfind, &find_info) != 0);

    FindClose(hfind);
    return result;
}

std::vector<std::string> list_subdirs(const std::string& path,
    const std::atomic<bool>* stop = nullptr)
{
    std::vector<std::string> dirs;
    std::string pattern = path + "\\*";
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return dirs;

    do {
        if (stop && *stop) break;

        if (_stricmp(ffd.cFileName, ".") == 0 || _stricmp(ffd.cFileName, "..") == 0) continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) dirs.push_back(path + "\\" + ffd.cFileName);

    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);
    return dirs;
}


int main(int argc, char* argv[])
{
    //std::string root_path =  std::filesystem::is_directory((std::filesystem::path)argv[1]) ? argv[1] : ".";
    std::string root_path;// = (argc > 1) ? argv[1] : "E:\\";
    std::string target_file;// = (argc > 2) ? argv[2] : "BaichingCV.pdf";

    std::vector<std::string> current_batch = {};

    switch (argc) {
    case 1:
        // No arguments
        current_batch = get_user_search_dirs();  // or your default logic
        target_file = "";
        std::cout << "Please enter arguments. EXAMPLE:\n" << "fdz [TARGET_PATH/DIRECTORY] [TARGET_PATH]" << std::endl;
        return 0;
        break;

    case 2:
        // One argument - could be path or filename
        // You need business logic to decide
        if (std::filesystem::is_directory(argv[1])) {
            current_batch = { argv[1] };
            target_file = "";
        }
        else {
            current_batch = get_user_search_dirs();;  // current directory
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

    BS::thread_pool pool;

    
    //if(root_path.empty()) current_batch = get_user_search_dirs();
    //else 
        //current_batch = {root_path};
    
    size_t level = 0;

    while (!found && !current_batch.empty()) {
        std::cout << "Level " << level
            << " – scanning " << current_batch.size()
            << " directories...\n";

        std::vector<std::string> next_batch;
        std::mutex next_mutex;

        for (const auto& dir : current_batch) {
            pool.detach_task([&, dir] {
                if (found) return;

                // 1) Search for the file in this directory
                auto maybe = find_file(dir, target_file, &found);
                if (maybe) {
                    std::lock_guard lock(result_mutex);
                    if (!found) {
                        found = true;
                        result_path = *maybe;
                    }
                }

                // 2) Collect subdirectories for next level
                if (!found) {
                    auto subdirs = list_subdirs(dir, &found);
                    if (!found) {
                        std::lock_guard lock(next_mutex);
                        next_batch.insert(next_batch.end(),
                            subdirs.begin(),
                            subdirs.end());
                    }
                }
                });
        }

        pool.wait();
        current_batch = std::move(next_batch);
        ++level;
    }

    if (found) {
        std::cout << "\n✅ Found: " << result_path
            << "\n   (at depth level " << level - 1 << ")\n";
    }
    else {
        std::cout << "\n❌ Not found.\n";
    }
}