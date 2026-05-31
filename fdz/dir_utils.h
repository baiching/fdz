#include <string>
#include <vector>
#include <unordered_set>
#include <Windows.h>
#include <array>
#include <shlobj.h> 
#pragma once
class Dir_Utils {
public:
	Dir_Utils();
	void lower(std::string& s);

	std::string filename_from_path(const std::string& fullpath);

	std::vector<std::string> get_fixed_drives();
	// Returns system-known folder path (e.g., Desktop, Documents) or empty string if failed
	std::string known_folder_path(const KNOWNFOLDERID& id);
	std::vector<std::string> get_user_search_dirs();
private:
	const std::unordered_set<std::string> SKIP_DIR_LIST;
};
