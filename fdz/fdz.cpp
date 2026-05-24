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
#include <rapidfuzz/distance/Levenshtein.hpp>
#include <unordered_set>
#include <string>

#pragma comment(lib, "Shlwapi.lib")

const std::unordered_set<std::string> SKIP_DIRS = {
    "node_modules",
    ".git",
    ".svn",
    "build",
    "cmake-build-debug",
    "cmake-build-release",
    "__pycache__",
    ".vs",
    ".idea",
    "vendor",
    "bower_components",
    "target",          // Rust
    "obj",             // .NET
    "bin",             // often build output
    ".cache",
    "dist",
    "out"
};

static void lower(std::string& s) {
    for (auto& c : s) c = (char)::tolower((unsigned char)c);
}

std::string filename_from_path(const std::string& fullpath) {
    auto pos = fullpath.rfind('\\');
    return (pos != std::string::npos) ? fullpath.substr(pos + 1) : fullpath;
}

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

std::vector<std::string> find_file(const std::string& path,
    const std::string& target,
    const std::atomic<bool>* stop = nullptr)
{
    std::string pattern = path + "\\*";
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &ffd);
    
    // Prepare lowercase query once (only need to do this per directory)
    std::string query = target;
    lower(query);

    std::vector<std::string> result;
    if (hFind == INVALID_HANDLE_VALUE) return result;

    do {
        if (stop && *stop) break;

        // Skip . and ..
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) continue;

        // Only interested in files
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        // ---------- fuzzy‑match the filename ----------
        std::string fname(ffd.cFileName);
        std::string fname_lower = fname;
        lower(fname_lower);

        bool matched = false;

        // Fast path 1: exact case‑insensitive match
        if (fname_lower == query) {
            matched = true;
        }
        // Fast path 2: substring match
        else if (fname_lower.find(query) != std::string::npos) {
            matched = true;
        }
        // Slow path 3: Levenshtein distance (via rapidfuzz)
        else {
            size_t dist = rapidfuzz::levenshtein_distance(query, fname_lower);
            int max_dist = std::min(2, (int)query.size() / 3);
            if (max_dist < 1) max_dist = 1;
            if (dist <= (size_t)max_dist)
                matched = true;
        }

        if (matched) {
            result.push_back(path + "\\" + ffd.cFileName);
        }
        
    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);
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

std::vector<std::string> search_all_files_bfs(
    const std::vector<std::string>& root,
    const std::string& target)
{
    BS::thread_pool pool;
    std::vector<std::string> all_matches;
    std::mutex result_mutex;
    std::vector<std::string> current_batch =  root;

    size_t level = 0;

    while (!current_batch.empty()) {
            std::cout << "Level " << level
            << " – scanning " << current_batch.size()
            << " directories...\n";
        std::vector<std::string> next_batch;
        std::mutex next_mutex;

        for (const auto& dir : current_batch) {
            pool.detach_task([&, dir] { 
                // 1) Fuzzy‑find all matches in this directory
                auto local = find_file(dir, target);
                if (!local.empty()) {
                    std::lock_guard lock(result_mutex);
                    all_matches.insert(all_matches.end(),
                        local.begin(), local.end());
                }

                // 2) Collect subdirectories (no stop flag needed)
                auto subdirs = list_subdirs(dir, nullptr);
                for (const auto& sub : subdirs) {
                    std::string fname = filename_from_path(sub);
                    for (auto& c : fname) c = (char)::tolower((unsigned char)c);
                    if (SKIP_DIRS.find(fname) == SKIP_DIRS.end()) {
                        std::lock_guard lock(next_mutex);
                        next_batch.push_back(sub);
                    }
                }
                });
        }

        pool.wait();                          // ★ BS API – barrier for this level
        current_batch = std::move(next_batch);
        ++level;
    }

    return all_matches;
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


    auto matches = search_all_files_bfs(current_batch, target_file);

    if (matches.empty()) {   // ← vector::empty() – correct!
        std::cout << "No files found.\n";
    }
    else {
        std::cout << "Found " << matches.size() << " file(s):\n";
        for (const auto& path : matches)
            std::cout << path << "\n";
    }
}