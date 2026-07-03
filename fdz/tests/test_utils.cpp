#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <functional>

// The header you provided
#include "../util/utils.h"   // adjust the include name

/*gtest section*/
TEST(ConvertallWcharToUtf8Test, ZeroArgs_NullArgv) {
    auto result = convertall_wchar_to_utf8(0, nullptr);
    EXPECT_TRUE(result.empty());
}

TEST(ConvertAllWcharToUtf8Test, SingleArg_ProgramNameOnly) {
    wchar_t arg1[] = { L"tool.exe" };
    LPWSTR argv[] = { arg1 };

    int argc = 1;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_STREQ(result[0].c_str(), "tool.exe");
}

TEST(ConvertAllWcharToUtf8Test, TypicalArgvWithMultipleArguments) {
    wchar_t arg0[] = L"myapp.exe";
    wchar_t arg1[] = L"-v";
    wchar_t arg2[] = L"file with spaces.txt";

    wchar_t* argv[] = { arg0, arg1, arg2 };
    int argc = 3;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "myapp.exe");
    EXPECT_EQ(result[1], "-v");
    EXPECT_EQ(result[2], "file with spaces.txt");
}

TEST(ConvertAllWcharToUtf8Test, UnicodeArgumentsCyrillicAndLatinSupplement) {
    wchar_t arg0[] = L"программа.exe";
    wchar_t arg1[] = L"--name=José" ;

    wchar_t* argv[] = { arg0, arg1 };
    int argc = 2;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "программа.exe");
    EXPECT_EQ(result[1], "--name=José");
}

TEST(ConvertAllWcharToUtf8Test, EmojiAndSurrogatePairs) {
    wchar_t arg0[] = L"app.exe";
    wchar_t arg1[] = L"🚀";
    wchar_t arg2[] = L"--flag";

    wchar_t* argv[] = { arg0, arg1, arg2 };
    int argc = 3;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "app.exe");
    // UTF-8 encoding of 🚀 is \xF0\x9F\x9A\x80
    EXPECT_EQ(result[1], "\xF0\x9F\x9A\x80");
    EXPECT_EQ(result[2], "--flag");
}

TEST(ConvertAllWcharToUtf8Test, EmptyStringElement) {
    wchar_t arg0[] = L"";
    wchar_t arg1[] = L"param";

    wchar_t* argv[] = { arg0, arg1 };
    int argc = 2;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "");   // empty
    EXPECT_EQ(result[1], "param");
}
TEST(ConvertAllWcharToUtf8Test, NullElementInArgv) {
    wchar_t arg0[] = L"prog";
    wchar_t arg2[] = L"end";

    wchar_t* argv[] = { arg0, nullptr, arg2 };
    int argc = 3;

    auto result = convertall_wchar_to_utf8(argc, argv);

    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "prog");
    // If NULL -> empty string or exception; we expect empty for safety.
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "end");
}

TEST(ConvertAllWcharToUtf8Test, DynamicArgvFromVector) {
    std::vector<std::wstring> args = { L"script.py", L"--input", L"data.csv", L"--verbose" };

    std::vector<wchar_t*> argv;

    for (const auto& arg : args) {
        argv.push_back(_wcsdup(arg.c_str()));  // allocates mutable copy
    }

    int argc = static_cast<int>(argv.size());

    auto result = convertall_wchar_to_utf8(argc, argv.data());

    // Don't forget to free!
    for (auto ptr : argv) {
        free(ptr);
    }

    ASSERT_EQ(result.size(), 4u);
    EXPECT_EQ(result[0], "script.py");
    EXPECT_EQ(result[1], "--input");
    EXPECT_EQ(result[2], "data.csv");
    EXPECT_EQ(result[3], "--verbose");
}
/* End of convertall tests */

// ======================================================================
// 2. Tests for utf8_to_wchar
// ======================================================================
TEST(Utf8ToWcharTest, BasicAsciiConversion) {
    const char input[] = "World";

    std::wstring result = utf8_to_wchar(input);

    ASSERT_EQ(result, L"World");
}
TEST(Utf8ToWcharTest, EmptyString) {
    const char input[] = "";

    std::wstring result = utf8_to_wchar(input);

    ASSERT_EQ(result, L"");
}
TEST(Utf8ToWcharTest, UnicodeCharacters) {
    const char input[] = "\xE2\x82\xAC \xE6\x97\xA5";

    std::wstring result = utf8_to_wchar(input);

    ASSERT_EQ(result, L"\u20AC \u65E5");
}
TEST(Utf8ToWcharTest, NullPointerInput) {
    std::wstring result = utf8_to_wchar(nullptr);

    ASSERT_EQ(result, L"");
}

// ======================================================================
// 3. Additional convertall_wchar_to_utf8 tests (original [convertall2] tag)
// ======================================================================
TEST(ConvertAllWcharToUtf8ExtraTest, TypicalArgvWithSpaces) {
    wchar_t arg0[] = L"myapp.exe";
    wchar_t arg1[] = L"-v";
    wchar_t arg2[] = L"file with spaces.txt";

    int argc = 3;
    wchar_t* argv[] = { arg0, arg1, arg2 };

    auto result = convertall_wchar_to_utf8(argc, argv);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "myapp.exe");
    EXPECT_EQ(result[1], "-v");
    EXPECT_EQ(result[2], "file with spaces.txt");
}
