#include "as_data_flow/as_data_flow.h"
#include "as_vector/as_vector.h"
#include "split/split.h"
#include "gmock/gmock.h"
#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(SplitTest, ByNewLine) {
    std::vector<std::stringstream> files(2);
    files[0] << "1\n2\n3\n4\n5";
    files[1] << "6\n7\n8\n9\n10";
    auto result = AsDataFlow(files) | Split("\n") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

TEST(SplitTest, BySpace) {
    std::vector<std::stringstream> files(2);
    files[0] << "1 2 3 4 5";
    files[1] << "6 7 8 9 10";
    auto result = AsDataFlow(files) | Split(" ") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

TEST(SplitTest, EmptyDelimiter) {
    std::vector<std::stringstream> files(1);
    files[0] << "abc";
    auto result = AsDataFlow(files) | Split("") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("a", "b", "c"));
}

TEST(SplitTest, EmptyStringsBetweenDelimiters) {
    std::vector<std::stringstream> files(1);
    files[0] << "a,,b,,,c";
    auto result = AsDataFlow(files) | Split(",") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("a", "", "b", "", "", "c"));
}

TEST(SplitTest, DelimitersAtStartAndEnd) {
    std::vector<std::stringstream> files(1);
    files[0] << "First\nSecond\nThird";
    auto result = AsDataFlow(files) | Split("\n") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("First", "Second", "Third"));
}

TEST(SplitTest, MixedEmptyAndNonEmptyFiles) {
    std::vector<std::stringstream> files(3);
    files[0] << "first";
    files[2] << "third";
    auto result = AsDataFlow(files) | Split(",") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("first", "", "third"));
}

TEST(SplitTest, EscapeCharactersAsDelimiters) {
    std::vector<std::stringstream> files(1);
    files[0] << "a.b.c";
    auto result = AsDataFlow(files) | Split(".") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("a", "b", "c"));
}

TEST(SplitTest, DelimetersNotFound) {
    std::vector<std::stringstream> files(1);
    files[0] << "a.b.c";
    auto result = AsDataFlow(files) | Split(",") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("a.b.c"));
}