#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include "../dir_utils.h"

uint32_t factorial(uint32_t number) {
    return number <= 1 ? number : factorial(number - 1) * number;
}

TEST_CASE("Checking if known_folder_path really works", "[known_folder_path]") {
	Dir_Utils ptr;

	REQUIRE(ptr.known_folder_path(FOLDERID_Desktop) != "");
	REQUIRE(ptr.known_folder_path(FOLDERID_Downloads) != "");
	REQUIRE(ptr.known_folder_path(FOLDERID_Documents) != "");
	REQUIRE(ptr.known_folder_path(FOLDERID_Pictures) != "");
	REQUIRE(ptr.known_folder_path(FOLDERID_Music) != "");
	REQUIRE(ptr.known_folder_path(FOLDERID_Videos) != "");
}

TEST_CASE("test get_fixed_drivers", "[get_fixed_drivers]") {
	Dir_Utils ptr;
	auto drives = ptr.get_fixed_drives();
	REQUIRE(!drives.empty());
	for (const auto& drive : drives) {
		REQUIRE(drive.size() == 3); // e.g., "C:\"
		REQUIRE(drive[1] == ':');
		REQUIRE(drive[2] == '\\');
	}
}

TEST_CASE("filename_from_path extracts filename", "[filename_from_path]") {
	Dir_Utils ptr;
	REQUIRE(ptr.filename_from_path("C:\\Users\\Alice\\file.txt") == "file.txt");
	REQUIRE(ptr.filename_from_path("C:\\path\\to\\directory\\") == "");
	REQUIRE(ptr.filename_from_path("file_only.txt") == "file_only.txt");
	REQUIRE(ptr.filename_from_path("") == "");
}

TEST_CASE("Factorials are computed", "[factorial]") {
    REQUIRE(factorial(1) == 1);
    REQUIRE(factorial(2) == 2);
    REQUIRE(factorial(3) == 6);
    REQUIRE(factorial(10) == 3'628'800);
}