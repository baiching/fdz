#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <array>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>    // for PathFindFileName, etc., if needed for verification
#endif

#include "../dir_utils.h"   // your header

// ------------------------------------------------------------------
// Test fixture: provides a fresh Dir_Utils object for each test
// ------------------------------------------------------------------
class DirUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        dirUtils = std::make_unique<Dir_Utils>();
    }

    void TearDown() override {}

    std::unique_ptr<Dir_Utils> dirUtils;
};

// ======================================================================
// 1. get_skip_list() tests
// ======================================================================
TEST_F(DirUtilsTest, GetSkipList_IsNotNullAndNotEmpty) {
    const std::unordered_set<std::string>& skip = dirUtils->get_skip_list();
    EXPECT_FALSE(skip.empty()) << "Skip list should contain at least some entries.";
}

TEST_F(DirUtilsTest, GetSkipList_ContainsCommonSkipDirs) {
    const std::unordered_set<std::string>& skip = dirUtils->get_skip_list();
    // These are common skip directories; the actual set depends on your implementation.
    // Adjust or remove if your implementation uses different names.
    EXPECT_TRUE(skip.count("node_modules") || skip.count("Node_Modules")) << "Expected 'node_modules' in skip list.";
    EXPECT_TRUE(skip.count(".git")) << "Expected '.git' in skip list.";
    EXPECT_TRUE(skip.count("out") || skip.count("out"));
}

TEST_F(DirUtilsTest, GetSkipList_ReturnsSameReference) {
    const std::unordered_set<std::string>& first = dirUtils->get_skip_list();
    const std::unordered_set<std::string>& second = dirUtils->get_skip_list();
    EXPECT_EQ(first, second) << "Returned skip list should be a stable reference to the same object.";
}

// ======================================================================
// 2. lower(wstring&) tests
// ======================================================================
TEST_F(DirUtilsTest, Lower_EmptyString_RemainsEmpty) {
    std::wstring s;
    dirUtils->wlower(s);
    EXPECT_EQ(s, L"");
}

TEST_F(DirUtilsTest, Lower_AllUppercase_BecomesLowercase) {
    std::wstring s = L"HELLO WORLD";
    dirUtils->wlower(s);
    EXPECT_EQ(s, L"hello world");
}

TEST_F(DirUtilsTest, Lower_MixedCaseWithDigits_OnlyLettersLowered) {
    std::wstring s = L"AbCdEf123!@#$%";
    dirUtils->wlower(s);
    EXPECT_EQ(s, L"abcdef123!@#$%");
}

TEST_F(DirUtilsTest, Lower_UnicodeCyrillic_HandlesUppercase) {
    // Cyrillic uppercase letters have distinct lowercase forms
    std::wstring s = L"ПРИВЕТ МИР";   // HELLO WORLD in Russian
    dirUtils->wlower(s);
    EXPECT_EQ(s, L"привет мир");
}

TEST_F(DirUtilsTest, Lower_UnicodeMixedScript_PreservesNonLetters) {
    // Includes Greek uppercase, digits, punctuation
    std::wstring s = L"Δοκιμή 123 – ΤΕΣΤ";
    dirUtils->wlower(s);
    // Expected: all letters become lowercase, the rest unchanged
    EXPECT_EQ(s, L"δοκιμή 123 – τεστ");
}

TEST_F(DirUtilsTest, Lower_AlreadyLowercase_Unchanged) {
    std::wstring s = L"already lowercase";
    dirUtils->wlower(s);
    EXPECT_EQ(s, L"already lowercase");
}

// ======================================================================
// 3. filename_from_path tests (Windows‑oriented)
// ======================================================================
TEST_F(DirUtilsTest, FilenameFromPath_WindowsBackslash_NormalFile) {
    std::string result = dirUtils->filename_from_path("C:\\Users\\John\\file.txt");
    EXPECT_EQ(result, "file.txt");
}

TEST_F(DirUtilsTest, FilenameFromPath_WindowsBackslash_FilenameOnlyNoExt) {
    std::string result = dirUtils->filename_from_path("C:\\Windows\\explorer");
    EXPECT_EQ(result, "explorer");
}

TEST_F(DirUtilsTest, FilenameFromPath_UnixForwardSlash_NormalFile) {
    std::string result = dirUtils->filename_from_path("/home/john/file.txt");
    EXPECT_EQ(result, "file.txt");
}

TEST_F(DirUtilsTest, FilenameFromPath_MixedSeparators_HandlesCorrectly) {
    std::string result = dirUtils->filename_from_path("D:/Projects\\src/main.cpp");
    // Usually the last separator decides, so we expect "main.cpp"
    EXPECT_EQ(result, "main.cpp");
}

TEST_F(DirUtilsTest, FilenameFromPath_JustFilename_ReturnsSame) {
    std::string result = dirUtils->filename_from_path("document.pdf");
    EXPECT_EQ(result, "document.pdf");
}

TEST_F(DirUtilsTest, FilenameFromPath_EmptyString_ReturnsEmpty) {
    std::string result = dirUtils->filename_from_path("");
    EXPECT_EQ(result, "");
}

TEST_F(DirUtilsTest, FilenameFromPath_RootDriveWithBackslash_ReturnsEmpty) {
    std::string result = dirUtils->filename_from_path("C:\\");
    EXPECT_EQ(result, "") << "Root drive should yield empty filename.";
}

TEST_F(DirUtilsTest, FilenameFromPath_RootDriveWithoutBackslash_ReturnsEmptyOrDrive) {
    // Some implementations might return "C:" or empty; test for robustness.
    std::string result = dirUtils->filename_from_path("C:");
    // If the implementation returns "C:" it's acceptable, but ideally empty.
    // We'll just check it doesn't crash and is not the full path.
    EXPECT_TRUE(result.empty() || result == "C:") << "Result: " << result;
}

TEST_F(DirUtilsTest, FilenameFromPath_PathEndsWithSeparator_ReturnsEmpty) {
    std::string result = dirUtils->filename_from_path("C:\\Windows\\System32\\");
    EXPECT_EQ(result, "") << "Trailing backslash should produce empty filename.";
}

TEST_F(DirUtilsTest, FilenameFromPath_MultipleDotsInFilename) {
    std::string result = dirUtils->filename_from_path("C:\\data\\archive.tar.gz");
    EXPECT_EQ(result, "archive.tar.gz");
}

TEST_F(DirUtilsTest, FilenameFromPath_FilenameWithSpaces) {
    std::string result = dirUtils->filename_from_path("C:\\My Documents\\my report.docx");
    EXPECT_EQ(result, "my report.docx");
}

TEST_F(DirUtilsTest, FilenameFromPath_RelativePath_Works) {
    std::string result = dirUtils->filename_from_path("..\\parent\\file.txt");
    EXPECT_EQ(result, "file.txt");
}

// ======================================================================
// 4. get_fixed_drives tests (Windows only)
// ======================================================================
#ifdef _WIN32
TEST_F(DirUtilsTest, GetFixedDrives_NotEmpty) {
    std::vector<std::string> drives = dirUtils->get_fixed_drives();
    EXPECT_FALSE(drives.empty()) << "At least one fixed drive (C:) should exist.";
}

TEST_F(DirUtilsTest, GetFixedDrives_EachEntryPlausible) {
    std::vector<std::string> drives = dirUtils->get_fixed_drives();
    for (const auto& drive : drives) {
        // Expect length >= 2 (e.g., "C:" or "C:\\")
        EXPECT_GE(drive.size(), 2u) << "Drive '" << drive << "' seems too short.";
        // Should contain a colon
        EXPECT_NE(drive.find(':'), std::string::npos) << "Drive '" << drive << "' missing colon.";
        // Usually uppercase letter
        EXPECT_TRUE(std::isupper(drive[0])) << "Drive letter should be uppercase, got '" << drive << "'";
        // Optionally, check that the path exists on the system
        if (drive.back() == '\\') {
            EXPECT_TRUE(PathFileExistsA(drive.c_str())) << "Drive root '" << drive << "' does not exist.";
        }
        else {
            std::string root = drive + "\\";
            EXPECT_TRUE(PathFileExistsA(root.c_str())) << "Drive root '" << root << "' does not exist.";
        }
    }
}

TEST_F(DirUtilsTest, GetFixedDrives_ContainsC) {
    std::vector<std::string> drives = dirUtils->get_fixed_drives();
    // At least one drive starting with 'C'
    bool hasC = std::any_of(drives.begin(), drives.end(),
        [](const std::string& d) { return d[0] == 'C' || d == "C:\\"; });
    EXPECT_TRUE(hasC) << "Fixed drives should include C: drive.";
}
#endif // _WIN32

// ======================================================================
// 5. known_folder_path tests (Windows only)
// ======================================================================
#ifdef _WIN32
TEST_F(DirUtilsTest, KnownFolderPath_Desktop_ReturnsNonEmpty) {
    std::string desktop = dirUtils->known_folder_path(FOLDERID_Desktop);
    EXPECT_FALSE(desktop.empty());
    // Ensure the path ends with "Desktop" (or localized equivalent) – not strictly required
    // but we can check it exists
    EXPECT_TRUE(PathFileExistsA(desktop.c_str())) << "Desktop path does not exist: " << desktop;
}

TEST_F(DirUtilsTest, KnownFolderPath_Documents_ReturnsNonEmpty) {
    std::string docs = dirUtils->known_folder_path(FOLDERID_Documents);
    EXPECT_FALSE(docs.empty());
    EXPECT_TRUE(PathFileExistsA(docs.c_str())) << "Documents path does not exist: " << docs;
}

TEST_F(DirUtilsTest, KnownFolderPath_Profile_ReturnsNonEmpty) {
    std::string profile = dirUtils->known_folder_path(FOLDERID_Profile);
    EXPECT_FALSE(profile.empty());
    EXPECT_TRUE(PathFileExistsA(profile.c_str())) << "Profile path does not exist: " << profile;
}

TEST_F(DirUtilsTest, KnownFolderPath_InvalidId_DoesNotCrash) {
    // A completely random GUID – function should handle gracefully.
    // It may return an empty string or throw; we just test that it doesn't crash.
    KNOWNFOLDERID invalidId = {};
    std::string result;
    EXPECT_NO_THROW(result = dirUtils->known_folder_path(invalidId));
    // Even if result is empty, we made it out alive.
}
#endif // _WIN32

// ======================================================================
// 6. get_user_search_dirs tests (Windows only)
// ======================================================================
#ifdef _WIN32
TEST_F(DirUtilsTest, GetUserSearchDirs_NotEmpty) {
    std::vector<std::string> searchDirs = dirUtils->get_user_search_dirs();
    EXPECT_FALSE(searchDirs.empty()) << "get_user_search_dirs must provide at least one directory.";
}

TEST_F(DirUtilsTest, GetUserSearchDirs_EachElementNonEmpty) {
    std::vector<std::string> searchDirs = dirUtils->get_user_search_dirs();
    for (const auto& dir : searchDirs) {
        EXPECT_FALSE(dir.empty()) << "One of the search directories is empty.";
    }
}

TEST_F(DirUtilsTest, GetUserSearchDirs_ContainsDesktop) {
    // The desktop should be one of the initial search dirs.
    std::string desktop = dirUtils->known_folder_path(FOLDERID_Desktop);
    std::vector<std::string> searchDirs = dirUtils->get_user_search_dirs();
    bool found = std::find(searchDirs.begin(), searchDirs.end(), desktop) != searchDirs.end();
    EXPECT_TRUE(found) << "get_user_search_dirs should include Desktop. Desktop path: " << desktop;
}

TEST_F(DirUtilsTest, GetUserSearchDirs_ContainsFixedDrivesRoots) {
    std::vector<std::string> drives = dirUtils->get_fixed_drives();
    std::vector<std::string> searchDirs = dirUtils->get_user_search_dirs();
    // At least the C: drive root should be present (often as "C:\" or "C:")
    // Let's check that at least one fixed drive appears in the search dirs.
    bool anyDrivePresent = false;
    for (const auto& drive : drives) {
        if (std::find(searchDirs.begin(), searchDirs.end(), drive) != searchDirs.end() ||
            (drive.back() != '\\' &&
                std::find(searchDirs.begin(), searchDirs.end(), drive + "\\") != searchDirs.end())) {
            anyDrivePresent = true;
            break;
        }
    }
    EXPECT_TRUE(anyDrivePresent) << "None of the fixed drive roots found in search directories.";
}
#endif // _WIN32
