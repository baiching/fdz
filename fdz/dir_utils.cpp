#include "dir_utils.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <array>
#include <Windows.h>

Dir_Utils::Dir_Utils() : SKIP_DIR_LIST({
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
	}) {
}

void Dir_Utils::lower(std::string& s)  {

	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); });

}
template<char... Seprators>
std::string Dir_Utils::filename_from_path(const std::string& fullpath) {
	constexpr std::array<char, 256> is_separator = []() {
		std::array<char, 256> arr{};
		((arr[static_cast<unsigned char>(Seprators)] = 1), ...);
		return arr;
		}();

	size_t last_slash = std::string::npos;
	for (size_t i = fullpath.size(); i > 0; --i) {
		if (is_separator[static_cast<unsigned char>(fullpath[i - 1])]) {
			last_slash = i - 1;
			break;
		}
	}

	if (last_slash == std::string::npos) return fullpath;

	return fullpath.substr(last_slash + 1);
}

std::vector<std::string> Dir_Utils::get_fixed_drives() {
	std::vector<std::string> drives;
	std::array<char, 512> buff{};
	DWORD len = GetLogicalDriveStringsA(buff.size(), buff.data());

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
	WideCharToMultiByte(CP_UTF8, 0, path, -1, rpath.data(), len, nullptr, nullptr);

	return rpath;
}