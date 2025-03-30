#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(WriteTest, Write) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    std::stringstream file_emulator;
    auto result = AsDataFlow(input) | Write(file_emulator, '|');
    ASSERT_EQ(file_emulator.str(), "1|2|3|4|5|");
}

TEST(WriteTest, WriteEmpty) {
    std::vector<int> input;
    std::stringstream file_emulator;
    AsDataFlow(input) | Write(file_emulator, ',');
    ASSERT_EQ(file_emulator.str(), "");
}

TEST(WriteTest, WriteStrings) {
    std::vector<std::string> input = {"Hello", "World", "Test"};
    std::stringstream file_emulator;
    AsDataFlow(input) | Write(file_emulator, ' ');
    ASSERT_EQ(file_emulator.str(), "Hello World Test ");
}

TEST(WriteTest, WriteFloats) {
    std::vector<float> input = {0.5f, 1.5f, 2.75f};
    std::stringstream file_emulator;
    AsDataFlow(input) | Write(file_emulator, '|');
    ASSERT_EQ(file_emulator.str(), "0.5|1.5|2.75|");
}

TEST(WriteTest, WriteNoDelimiter) {
    std::vector<int> input = {11, 22, 33};
    std::stringstream file_emulator;
    AsDataFlow(input) | Write(file_emulator);
    ASSERT_EQ(file_emulator.str(), "112233");
}