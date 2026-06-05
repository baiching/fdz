#pragma once
#include "./BS_thread_pool.hpp"
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

    std::vector<std::string> concurrent_search(
        const std::vector<std::string> &roots,
        std::string &target);
private:
};