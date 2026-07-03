#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <array>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h> 
#endif

class Dir_Utils {
public:
	Dir_Utils();

	const std::unordered_set<std::string> get_skip_list() const;

	// to convert a string to lowercase 
	// or to facilaitate fuzzy matching via rapidfuzz
	void wlower(std::wstring& s);

	// to extract filename from a full path
	std::string filename_from_path(const std::string& fullpath);

	// to get all fixed drives 
	std::vector<std::string> get_fixed_drives();

	// to get paths from C drive without 
	// invoking full scan on entire drive
	std::string known_folder_path(const KNOWNFOLDERID& id);

	// to get a good initial set of 
	// directories to search (e.g., Desktop, 
	// Documents, plus all fixed drives)
	std::vector<std::string> get_user_search_dirs();
private:
	// just a set of common directory names to skip during traversal
	const std::unordered_set<std::string> SKIP_DIR_LIST;
};
