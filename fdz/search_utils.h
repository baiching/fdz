#pragma once
#include "BS_thread_pool.hpp"
#include "dir_utils.h"
#include "concurrentqueue.h"
#include "task_queue.h"
#include <string>
#include <vector>
#include <atomic>

#ifdef _WIN32
    #define COLOR_RESET   "\033[0m"         //reset
    #define COLOR_DIR     "\033[38;5;117m"  //light blue
    #define COLOR_REPARSE "\033[32m"        //green
    #define COLOR_HIDDEN  "\033[90m"        //dark_grey
    #define COLOR_FILE    "\033[93m"        //yellow
#else
    #define COLOR_RESET   "\033[0m"
    #define COLOR_DIR     "\033[34m"
    #define COLOR_REPARSE "\033[33m"
    #define COLOR_HIDDEN  "\033[90m"
    #define COLOR_FILE    "\033[32m"
#endif // _WIN32


extern moodycamel::ConcurrentQueue<std::string> result_q;

class Search_Utils {
public:
    void process_directories(std::unique_ptr<batch_s> batch, std::string& target, Search_Utils& srh, size_t batch_size);

    void find_file(const std::string& path,
        const std::string& target,
        const std::atomic<bool>* stop,
        std::vector<std::string>& dirs,
        Dir_Utils& dir_utils,
        std::unordered_set<std::string>& skip_list);

    //it calls process_directories
    void concurrent_search(
        const std::vector<std::string> &roots,
        std::string &target);
private:
    
};