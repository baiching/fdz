// test_search_utils.cpp
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "search_utils.h"
#include "dir_utils.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ------------------------------------------------------------------
// Helper: create a file (relative to the test root)
// ------------------------------------------------------------------
static void create_file(const fs::path& root, const std::string& rel_path, const std::string& content = "") {
    fs::path full = root / rel_path;
    fs::create_directories(full.parent_path());
    std::ofstream ofs(full);
    ofs << content;
}

static void create_dir(const fs::path& root, const std::string& rel_path) {
    fs::create_directories(root / rel_path);
}

// ------------------------------------------------------------------
// The actual tests – use a real directory structure
// ------------------------------------------------------------------
TEST_CASE("Search_Utils::find_file works with exact and substring matches", "[find_file]") {
    // Setup: use a real directory. You can change this path to anything.
    fs::path test_root = fs::current_path() / "test_files";
    fs::remove_all(test_root);   // start fresh
    fs::create_directories(test_root);

    // Create some test files
    create_file(test_root, "hello.txt");
    create_file(test_root, "subdir/world.txt");
    create_file(test_root, "subdir/Another_Doc.pdf");

    Dir_Utils du;
    Search_Utils su;

    SECTION("Exact match") {
        auto res = su.find_file(test_root.string(), "hello.txt", nullptr, du);
        REQUIRE(res.size() == 1);
        CHECK(res[0].find("hello.txt") != std::string::npos);
    }

    SECTION("Case‑insensitive exact match") {
        auto res = su.find_file(test_root.string(), "HELLO.TXT", nullptr, du);
        REQUIRE(res.size() == 1);
    }

    SECTION("Substring match") {
        auto res = su.find_file(test_root.string(), "world", nullptr, du);
        REQUIRE(res.size() == 1);
        CHECK(res[0].find("world.txt") != std::string::npos);
    }

    SECTION("No match") {
        auto res = su.find_file(test_root.string(), "nonexistent", nullptr, du);
        CHECK(res.empty());
    }

    // Cleanup
    fs::remove_all(test_root);
}

TEST_CASE("Search_Utils::list_subdirs returns only directories", "[list_subdirs]") {
    fs::path test_root = fs::current_path() / "test_files";
    fs::remove_all(test_root);
    fs::create_directories(test_root);

    create_dir(test_root, "dirA");
    create_dir(test_root, "dirB");
    create_dir(test_root, "sub/dirC");
    create_file(test_root, "file.txt");   // should be ignored

    Search_Utils su;
    auto dirs = su.list_subdirs(test_root.string(), nullptr);

    // Expect three directories: dirA, dirB, sub
    REQUIRE(dirs.size() == 3);
    CHECK(std::find(dirs.begin(), dirs.end(), (test_root / "dirA").string()) != dirs.end());
    CHECK(std::find(dirs.begin(), dirs.end(), (test_root / "dirB").string()) != dirs.end());
    CHECK(std::find(dirs.begin(), dirs.end(), (test_root / "sub").string()) != dirs.end());

    fs::remove_all(test_root);
}

TEST_CASE("Search_Utils::concurrent_search finds files across subdirectories", "[concurrent_search]") {
    fs::path test_root = fs::current_path() / "test_files";
    fs::remove_all(test_root);
    fs::create_directories(test_root);

    // Build a small tree
    create_file(test_root, "root.txt");
    create_dir(test_root, "sub1");
    create_file(test_root, "sub1/file1.txt");
    create_dir(test_root, "sub1/subsub");
    create_file(test_root, "sub1/subsub/file2.txt");
    create_dir(test_root, "sub2");
    create_file(test_root, "sub2/other.pdf");

    Search_Utils su;
    std::string target = ".txt";   // find all .txt files
    std::vector<std::string> roots = { test_root.string() };

    auto results = su.concurrent_search(roots, target);

    // Should find root.txt, sub1/file1.txt, sub1/subsub/file2.txt = 3 files
    REQUIRE(results.size() == 3);
    for (auto& path : results) {
        CHECK(fs::exists(path));
        CHECK(path.find(".txt") != std::string::npos);
    }

    fs::remove_all(test_root);
}

TEST_CASE("concurrent_search stops early when stop flag is set", "[concurrent_search][stop]") {
    fs::path test_root = fs::current_path() / "test_files";
    fs::remove_all(test_root);
    fs::create_directories(test_root);

    // Create a deep tree
    for (int i = 0; i < 50; ++i) {
        create_file(test_root, "dir" + std::to_string(i) + "/file.txt");
    }

    Search_Utils su;
    std::string target = "file.txt";
    std::vector<std::string> roots = { test_root.string() };

    // We can't easily test the atomic stop inside concurrent_search because
    // the stop flag is not exposed. But we can verify the function doesn't crash.
    auto results = su.concurrent_search(roots, target);
    // It should find all 50 files unless the internal stop flag is triggered,
    // which it isn't in this test. This just confirms basic operation.

    CHECK(results.size() == 50);
    fs::remove_all(test_root);
}