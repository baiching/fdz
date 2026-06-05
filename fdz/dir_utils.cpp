#include "dir_utils.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

Dir_Utils::Dir_Utils() : SKIP_DIR_LIST({
    // VCS
    ".git",
    ".svn",
    ".hg",

    // MSVC / Visual Studio
    ".vs",
    "ipch",
    "Debug",
    "Release",
    "RelWithDebInfo",
    "MinSizeRel",
    "x64",
    "Win32",
    "ARM64",
    "ARM",

    // CMake / build systems
    "build",
    "cmake-build-debug",
    "cmake-build-release",
    "CMakeFiles",
    "_deps",

    // Dependencies / third-party
    "node_modules",
    "vendor",
    "bower_components",
    "third_party",
    "third-party",
    "deps",
    "packages",
    "external",

    // Java / Maven / Gradle (Lucene, Solr, etc.)
    "target",
    "classes",
    "test-classes",
    ".gradle",
    "gradle",

    // Qt generated
    "moc",
    "rcc",
    "uic",
    ".rcc",

    // .NET / C#
    "obj",
    "bin",

    // Python
    "__pycache__",
    ".mypy_cache",
    ".pytest_cache",
    ".venv",
    "venv",
    "env",

    // Rust
    "target",

    // General build / output
    "out",
    "dist",
    ".cache",
    "coverage",
    "htmlcov",
    "test_output",
    "logs",
    "tmp",
    "temp",
    "backup",

    // IDE
    ".idea",
    ".vscode",
    ".eclipse",

    // system dirs
    "$recycle.bin"
	}) {
}

std::unordered_set<std::string> Dir_Utils::get_skip_list() {
	return this->SKIP_DIR_LIST;
}

void Dir_Utils::lower(std::wstring& s)  {

	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); });

}
std::string Dir_Utils::filename_from_path(const std::string& fullpath) {
	const std::string separators = "/\\";  // forward slash and backslash

	size_t last_slash = fullpath.find_last_of(separators);

	if (last_slash == std::string::npos) return fullpath;

	return fullpath.substr(last_slash + 1);
}

std::vector<std::string> Dir_Utils::get_fixed_drives() {
	std::vector<std::string> drives;
	std::array<char, 512> buff{};
	DWORD len = GetLogicalDriveStringsA(static_cast<DWORD>(buff.size()), buff.data());

	if (len == 0 || len > buff.size()) return drives;

	auto p = buff.data();
	while (*p) {
		if (GetDriveTypeA(p) == DRIVE_FIXED) 
			drives.push_back(p);
		p += std::strlen(p) + 1;
	}
	return drives;
}

std::string Dir_Utils::known_folder_path(const KNOWNFOLDERID& id) {
	PWSTR path = nullptr;
	if (FAILED(SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &path))) {
		return "";
	}
	
	auto guard = std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)>(path, CoTaskMemFree);

	int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);

	if (len <= 0) {
		return "";
	}

	std::string rpath(len - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, path, -1, &rpath[0], len, nullptr, nullptr);

	return rpath;
}

std::vector<std::string> Dir_Utils::get_user_search_dirs() {
	std::vector<std::string> dirs;
	const KNOWNFOLDERID search_folders[] = {
		FOLDERID_Desktop,
		FOLDERID_Downloads,
		FOLDERID_Documents,
		FOLDERID_Pictures,
		FOLDERID_Music,
		FOLDERID_Videos
	};

	for (auto id : search_folders) {
		std::string path = known_folder_path(id);
		if (!path.empty() && GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
			dirs.push_back(path);
		}
	}

	auto rest = this->get_fixed_drives();
	rest.erase(std::remove(rest.begin(), rest.end(), "C:\\"), rest.end());

	dirs.insert(dirs.end(), rest.begin(), rest.end());
	return dirs;
}