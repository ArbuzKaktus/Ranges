#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

TEST(DropNulloptTest, DropNullopt) {
    std::vector<std::optional<int>> input = {1, std::nullopt, 3, std::nullopt, 5};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 3, 5));
}

TEST(DropNulloptTest, EmptyVector) {
    std::vector<std::optional<int>> input = {};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(DropNulloptTest, AllNullopt) {
    std::vector<std::optional<int>> input = {std::nullopt, std::nullopt, std::nullopt};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(DropNulloptTest, NoNullopt) {
    std::vector<std::optional<int>> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(DropNulloptTest, StringType) {
    std::vector<std::optional<std::string>> input = {
        "apple", 
        std::nullopt, 
        "banana", 
        std::nullopt, 
        "cherry"
    };
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("apple", "banana", "cherry"));
}

TEST(DropNulloptTest, CustomType) {
    struct Person {
        std::string name;
        int age;
        
        bool operator==(const Person& other) const {
            return name == other.name && age == other.age;
        }
    };
    
    std::vector<std::optional<Person>> input = {
        Person{"Alice", 25},
        std::nullopt,
        Person{"Bob", 30},
        std::nullopt
    };
    
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0].name, "Alice");
    ASSERT_EQ(result[0].age, 25);
    ASSERT_EQ(result[1].name, "Bob");
    ASSERT_EQ(result[1].age, 30);
}

TEST(DropNulloptTest, NulloptAtBoundaries) {
    std::vector<std::optional<int>> input = {std::nullopt, 1, 2, 3, std::nullopt};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3));
}

TEST(DropNulloptTest, ConsecutiveNullopt) {
    std::vector<std::optional<int>> input = {1, std::nullopt, std::nullopt, std::nullopt, 5};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 5));
}

TEST(DropNulloptTest, CombineWithOtherAdapters) {
    std::vector<std::optional<int>> input = {1, std::nullopt, 3, std::nullopt, 5, 6, std::nullopt, 8};
    
    auto result = AsDataFlow(input)
        | DropNullopt()
        | Filter([](int x) { return x % 2 == 0; })
        | AsVector();
    
    ASSERT_THAT(result, testing::ElementsAre(6, 8));
}

TEST(DropNulloptTest, ResultType) {
    std::vector<std::optional<double>> input = {1.5, std::nullopt, 3.7};
    auto result = AsDataFlow(input) | DropNullopt() | AsVector();
    
    ASSERT_TRUE((std::is_same_v<decltype(result), std::vector<double>>));
    ASSERT_THAT(result, testing::ElementsAre(1.5, 3.7));
}