#include <string>
#include <vector>
#include <unordered_set>
#include <Windows.h>
#include <shlobj.h> 
#pragma once
class Dir_Utils {
public:
	Dir_Utils();
	void lower(std::string& s);
	template<char... Seprators>
	std::string filename_from_path(const std::string& fullpath);
	std::vector<std::string> get_fixed_drives();
	std::string known_folder_path(const KNOWNFOLDERID& id);
	std::string get_user_search_dirs();
private:
	const std::unordered_set<std::string> SKIP_DIR_LIST;
};
