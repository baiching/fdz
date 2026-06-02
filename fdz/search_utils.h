#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <rapidfuzz/distance/Levenshtein.hpp>

class Search_Utils {
public:
    std::vector<std::string> find_file(const std::string& path,
        const std::string& target,
        const std::atomic<bool>* stop = nullptr);

    std::vector<std::string> list_subdirs(const std::string& path,
        const std::atomic<bool>* stop = nullptr);

    std::vector<std::string> search_all_files_bfs(
        const std::vector<std::string>& root,
        const std::string& target);
private:
};