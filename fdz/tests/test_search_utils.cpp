#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "../search_utils.h"

TEST_CASE("List Subduirectories", "[list_subdirs]") {
	Search_Utils ptr;
	auto dirs = ptr.list_subdirs("C:\\", nullptr);
	REQUIRE(!dirs.empty());
	for (const auto& dir : dirs) {
		REQUIRE(dir.size() > 3); // e.g., "C:\Folder"
		REQUIRE(dir[1] == ':');
		REQUIRE(dir[2] == '\\');
		std::cout << dir << std::endl;
	}

	//for (auto& dir : dirs) {
	//	std::cout << dir << std::endl;
	//}
}