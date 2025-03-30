#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(TransformTest, PowerOfTwo) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Transform([](int x) { return x * x; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 4, 9, 16, 25));
}

TEST(TransformTest, FromStringToInt) {
    std::vector<std::stringstream> files(2);
    files[0] << "1 2 3 4 5";
    files[1] << "6 7 8 9 10";
    auto result = AsDataFlow(files) | Split(" ") | Transform([](const std::string& str) { return std::stoi(str); }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

TEST(TransformTest, UppercaseStrings) {
    std::vector<std::string> input = {"abc", "Test", "data"};
    auto result = AsDataFlow(input)
        | Transform([](const std::string &s) {
            std::string out = s;
            for (auto &ch : out) ch = static_cast<char>(std::toupper(ch));
            return out;
        })
        | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("ABC", "TEST", "DATA"));
}

TEST(TransformTest, IncrementFloats) {
    std::vector<float> input = {1.5f, 2.5f, 3.5f};
    auto result = AsDataFlow(input)
        | Transform([](float x) { return x + 1.0f; })
        | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(2.5f, 3.5f, 4.5f));
}

TEST(TransformTest, ConcatStrings) {
    std::vector<std::string> input = {"hello", "world"};
    auto result = AsDataFlow(input)
        | Transform([](const std::string &s) {
            return s + "_suffix";
        })
        | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("hello_suffix", "world_suffix"));
}

TEST(TransformTest, ZeroElements) {
    std::vector<int> empty;
    auto result = AsDataFlow(empty)
        | Transform([](int x) { return x * 2; })
        | AsVector();
    ASSERT_TRUE(result.empty());
}