#pragma once
#include "BS_thread_pool.hpp"
#include "dir_utils.h"
#include "concurrentqueue.h"
#include <string>
#include <vector>
#include <atomic>

extern moodycamel::ConcurrentQueue<std::string> result_q;

class Search_Utils {
public:
    void find_file(const std::string& path,
        const std::string& target,
        const std::atomic<bool>* stop,
        std::vector<std::string>& dirs,
        Dir_Utils& dir_utils);

    std::vector<std::string> list_subdirs(const std::string& path,
        const std::atomic<bool>* stop = nullptr);

    void concurrent_search(
        const std::vector<std::string> &roots,
        std::string &target);
private:
    
};