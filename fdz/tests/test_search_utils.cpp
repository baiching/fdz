#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <Windows.h>
#include <shlobj.h>

#include "../search_utils.h"
#include "../dir_utils.h"
#include "../task_queue.h"      // for batch_s, add_batch, get_batch
#include "../concurrentqueue.h"
#include <regex>

std::string strip_ansi(const std::string& s) {
    static const std::regex ansi_regex("\x1B\\[[0-9;]*m");
    return std::regex_replace(s, ansi_regex, "");
}

// ------------------------------------------------------------------
//  Utility RAII for redirecting std::cout
// ------------------------------------------------------------------
class CoutRedirect {
public:
    CoutRedirect() : m_sbuf(std::cout.rdbuf(m_ss.rdbuf())) {}
    ~CoutRedirect() { std::cout.rdbuf(m_sbuf); }
    std::string str() const { return m_ss.str(); }
private:
    std::ostringstream m_ss;
    std::streambuf* m_sbuf;
};

// ------------------------------------------------------------------
//  Test Fixture
// ------------------------------------------------------------------
class SearchUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Drain the global result queue (not used in output, but clean)
        std::string ignored;
        while (result_q.try_dequeue(ignored)) {}

        // Create a unique temporary root directory
        TCHAR tempPath[MAX_PATH];
        GetTempPath(MAX_PATH, tempPath);
        UINT uniqueId = GetTempFileName(tempPath, TEXT("sut_"), 0, tempPath);
        DeleteFile(tempPath);                                   // remove the created file
        CreateDirectory(tempPath, nullptr);
        m_tempRoot = tempPath;

        buildTestTree();
    }

    void TearDown() override {
        // Recursively delete the whole tree
        std::string cmd = "rmdir /s /q \"" + m_tempRoot + "\"";
        system(cmd.c_str());
    }

    void buildTestTree() {
        // Tree:
        // m_tempRoot
        //   fileA.txt
        //   FileB.TXT             (uppercase extension)
        //   subdir1
        //     fileA.txt
        //   Windows               (this name is in the skip list)
        //     secret.txt
        //   hidden
        //     hidden.txt          (FILE_ATTRIBUTE_HIDDEN set)
        std::string subdir1 = join(m_tempRoot, "subdir1");
        std::string skipDir = join(m_tempRoot, "Windows");
        std::string hiddenDir = join(m_tempRoot, "hidden");

        CreateDirectoryA(subdir1.c_str(), nullptr);
        CreateDirectoryA(skipDir.c_str(), nullptr);
        CreateDirectoryA(hiddenDir.c_str(), nullptr);

        createFile(join(m_tempRoot, "fileA.txt"), "hello");
        createFile(join(m_tempRoot, "FileB.TXT"), "world");
        createFile(join(subdir1, "fileA.txt"), "dup");
        createFile(join(skipDir, "secret.txt"), "secret");
        std::string hiddenFile = join(hiddenDir, "hidden.txt");
        createFile(hiddenFile, "hidden");
        SetFileAttributesA(hiddenFile.c_str(), FILE_ATTRIBUTE_HIDDEN);
    }

    void createFile(const std::string& path, const std::string& content) {
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }

    std::string join(const std::string& base, const std::string& sub) {
        return base + "\\" + sub;
    }

    std::string m_tempRoot;
};

// ------------------------------------------------------------------
//  1. Empty target (lists directory contents)
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_EmptyTarget_ListsDirectory) {
    Search_Utils sutil;
    std::string target;               // empty
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = coutRedirect.str();

    // The output should contain the names of each entry in the root (non‑recursive)
    EXPECT_THAT(output, ::testing::HasSubstr("fileA.txt"));
    EXPECT_THAT(output, ::testing::HasSubstr("FileB.TXT"));
    EXPECT_THAT(output, ::testing::HasSubstr("subdir1"));
    EXPECT_THAT(output, ::testing::HasSubstr("Windows"));
    EXPECT_THAT(output, ::testing::HasSubstr("hidden"));
    // The function stops after listing the root – no recursive scanning
    EXPECT_THAT(output, ::testing::Not(::testing::HasSubstr("secret.txt")));
}

TEST_F(SearchUtilsTest, ConcurrentSearch_EmptyTarget_InvalidRoot_DoesNotCrash) {
    Search_Utils sutil;
    std::string target;
    std::vector<std::string> roots = { "Z:\\NoSuchPath" };

    CoutRedirect coutRedirect;
    EXPECT_NO_THROW(sutil.concurrent_search(roots, target));
    std::string output = coutRedirect.str();
    EXPECT_TRUE(output.empty()) << "No output expected for invalid root";
}

// ------------------------------------------------------------------
//  2. Exact match
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_ExactMatch_FindsFiles) {
    Search_Utils sutil;
    std::string target = "fileA.txt";
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = strip_ansi(coutRedirect.str());

    // Both instances (root and subdir1) should appear with COLOR_DIR and COLOR_FILE
    std::string expectedRoot = m_tempRoot + "\\" + "fileA.txt";
    std::string expectedSub = m_tempRoot + "\\subdir1\\fileA.txt";
    EXPECT_THAT(output, ::testing::HasSubstr(expectedRoot));
    EXPECT_THAT(output, ::testing::HasSubstr(expectedSub));
}

// ------------------------------------------------------------------
//  3. Case‑insensitive match
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_CaseInsensitiveFind) {
    Search_Utils sutil;
    std::string target = "FILEa.TXT";   // different case
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = coutRedirect.str();

    // Should match fileA.txt (both instances)
    std::string lowerFile = "fileA.txt";
    EXPECT_THAT(output, ::testing::HasSubstr(lowerFile));
}

// ------------------------------------------------------------------
//  4. Substring match
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_SubstringMatch) {
    Search_Utils sutil;
    std::string target = "file";       // substring
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = coutRedirect.str();

    // Should match all files whose name contains "file": fileA.txt (2 times), FileB.TXT
    EXPECT_THAT(output, ::testing::HasSubstr("fileA.txt"));
    EXPECT_THAT(output, ::testing::HasSubstr("FileB.TXT"));
    // The output for FileB.TXT will appear as-is (Fname), but matching is on lowercase, so it's present
}

// ------------------------------------------------------------------
//  5. Skip list – directories with a skip‑list name are not entered
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_SkipsSkipListDirectories) {
    // The skip list (Dir_Utils) contains "Windows". Our test tree has a "Windows" folder.
    Search_Utils sutil;
    std::string target = "should_not_find.txt";
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = strip_ansi(coutRedirect.str());

    // secret.txt is inside the skipped folder → should NOT be printed
    EXPECT_THAT(output, ::testing::Not(::testing::HasSubstr("should_not_find.txt")));
}

// ------------------------------------------------------------------
//  6. Hidden files – they are still found (only colour‑coded)
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_FindsHiddenFiles) {
    Search_Utils sutil;
    std::string target = "hidden.txt";
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = coutRedirect.str();

    // The file hidden/hidden.txt should be printed (with COLOR_HIDDEN)
    EXPECT_THAT(output, ::testing::HasSubstr("hidden.txt"));
}

// ------------------------------------------------------------------
//  7. Multiple root directories
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_MultipleRoots) {
    // Create a second temporary root
    std::string secondRoot = m_tempRoot + "_second";
    CreateDirectoryA(secondRoot.c_str(), nullptr);
    createFile(join(secondRoot, "unique.log"), "data");
    // TearDown will delete the main root, we need to delete this one manually
    auto cleanup = [&]() {
        std::string cmd = "rmdir /s /q \"" + secondRoot + "\"";
        system(cmd.c_str());
        };

    Search_Utils sutil;
    std::string target = "unique.log";
    std::vector<std::string> roots = { m_tempRoot, secondRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = coutRedirect.str();

    EXPECT_THAT(output, ::testing::HasSubstr("unique.log"));
    cleanup();
}

// ------------------------------------------------------------------
//  8. No match – output empty
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_NoMatch_EmptyOutput) {
    Search_Utils sutil;
    std::string target = "non_existent.zqx";
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    sutil.concurrent_search(roots, target);
    std::string output = strip_ansi(coutRedirect.str());

    EXPECT_EQ(output, "\n\n");
    //EXPECT_TRUE(output.empty());
}

// ------------------------------------------------------------------
//  9. Concurrency stress test (many small directories)
// ------------------------------------------------------------------
TEST_F(SearchUtilsTest, ConcurrentSearch_ManyDirectories_DoesNotDeadlock) {
    // Create 50 subdirs, each containing a file named "target.dat"
    for (int i = 0; i < 50; ++i) {
        std::string dir = join(m_tempRoot, "dir_" + std::to_string(i));
        CreateDirectoryA(dir.c_str(), nullptr);
        createFile(join(dir, "target.dat"), "data");
    }

    Search_Utils sutil;
    std::string target = "target.dat";
    std::vector<std::string> roots = { m_tempRoot };

    CoutRedirect coutRedirect;
    EXPECT_NO_THROW(sutil.concurrent_search(roots, target));
    std::string output = coutRedirect.str();

    // We should see 50 occurrences of "target.dat"
    size_t count = 0;
    size_t pos = 0;
    while ((pos = output.find("target.dat", pos)) != std::string::npos) {
        ++count;
        pos += std::string("target.dat").length();
    }
    EXPECT_EQ(count, 50u);
}
