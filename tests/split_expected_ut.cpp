#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expected>
#include <string>
#include <vector>

struct Department {
    std::string name;

    bool operator==(const Department& other) const {
        return name == other.name;
    }
};

std::expected<Department, std::string> ParseDepartment(const std::string& str) {
    if (str.empty()) {
        return std::unexpected("Department name is empty");
    }
    if (str.contains(' ')) {
        return std::unexpected("Department name contains space");
    }
    return Department{str};
}

std::expected<int, std::string> ParseInteger(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (const std::invalid_argument&) {
        return std::unexpected("Invalid number format");
    } catch (const std::out_of_range&) {
        return std::unexpected("Number out of range");
    }
}

TEST(SplitExpectedTest, SplitExpected) {
    std::vector<std::stringstream> files(1);
    files[0] << "good-department|bad department||another-good-department";

    auto [unexpected_flow, good_flow] = AsDataFlow(files) | Split("|") | Transform(ParseDepartment) | SplitExpected(ParseDepartment);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, '.');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Department name contains space.Department name is empty.");
    ASSERT_THAT(expected_result, testing::ElementsAre(Department{"good-department"}, Department{"another-good-department"}));
}

TEST(SplitExpectedTest, IntegerParsingTest) {
    std::vector<std::stringstream> files(1);
    files[0] << "123,abc,456,9999999999999999999,0,-42";

    auto [unexpected_flow, good_flow] = 
        AsDataFlow(files) | Split(",") | Transform(ParseInteger) | SplitExpected(ParseInteger);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, '|');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Invalid number format|Number out of range|");
    ASSERT_THAT(expected_result, testing::ElementsAre(123, 456, 0, -42));
}

TEST(SplitExpectedTest, AllSuccessfulTest) {
    std::vector<std::stringstream> files(1);
    files[0] << "dept1|dept2|dept3|dept4";

    auto [unexpected_flow, good_flow] = 
        AsDataFlow(files) | Split("|") | Transform(ParseDepartment) | SplitExpected(ParseDepartment);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, '.');

    auto expected_result = good_flow | AsVector();

    ASSERT_TRUE(unexpected_file.str().empty());
    ASSERT_THAT(expected_result, testing::ElementsAre(
        Department{"dept1"}, 
        Department{"dept2"}, 
        Department{"dept3"}, 
        Department{"dept4"}
    ));
}

TEST(SplitExpectedTest, AllFailedTest) {
    std::vector<std::stringstream> files(1);
    files[0] << "bad dept||invalid dept|another bad";

    auto [unexpected_flow, good_flow] = 
        AsDataFlow(files) | Split("|") | Transform(ParseDepartment) | SplitExpected(ParseDepartment);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, '.');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Department name contains space.Department name is empty.Department name contains space.Department name contains space.");
    ASSERT_TRUE(expected_result.empty());
}

TEST(SplitExpectedTest, MultipleFilesTest) {
    std::vector<std::stringstream> files(3);
    files[0] << "dept1|invalid dept";
    files[1] << "dept2|";
    files[2] << "bad dept|dept3";

    auto [unexpected_flow, good_flow] = 
        AsDataFlow(files) | Split("|") | Transform(ParseDepartment) | SplitExpected(ParseDepartment);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, ',');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Department name contains space,Department name is empty,Department name contains space,");
    ASSERT_THAT(expected_result, testing::ElementsAre(
        Department{"dept1"},
        Department{"dept2"},
        Department{"dept3"}
    ));
}

TEST(SplitExpectedTest, CustomTransformerTest) {
    std::vector<std::string> input = {"10", "-5", "abc", "15", "999999999999999999"};

    auto transformer = [](const std::string& str) -> std::expected<int, std::string> {
        try {
            int value = std::stoi(str);
            if (value <= 0) {
                return std::unexpected("Number must be positive");
            }
            return value;
        } catch (const std::exception&) {
            return std::unexpected("Parsing error");
        }
    };

    auto [unexpected_flow, good_flow] = 
        AsDataFlow(input) | Transform(transformer) | SplitExpected(transformer);

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, ' ');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Number must be positive Parsing error Parsing error ");
    ASSERT_THAT(expected_result, testing::ElementsAre(10, 15));
}