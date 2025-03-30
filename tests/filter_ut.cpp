#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

TEST(FilterTest, FilterEven) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int x) { return x % 2 == 0; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(2, 4));
}

TEST(FilterTest, FilterUpperCase) {
    std::vector<std::string> input = {"hello", "world", "HELLO", "WORLD"};
    auto result =
        AsDataFlow(input)
            | Filter([](const std::string& x) { return std::all_of(x.begin(), x.end(), [](char c) { return std::isupper(c); }); })
            | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("HELLO", "WORLD"));
}

//my tests
TEST(FilterTest, FilterEmptyVector) {
    std::vector<int> input = {};
    auto result = AsDataFlow(input) | Filter([](int x) { return x > 0; }) | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(FilterTest, FilterAllReject) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int) { return false; }) | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(FilterTest, FilterAllAccept) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int) { return true; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5));
    ASSERT_EQ(result.size(), input.size());
}

TEST(FilterTest, FilterByComplexCondition) {
    struct Person {
        std::string name;
        int age;
        bool is_student;
        
        bool operator==(const Person& other) const {
            return name == other.name && age == other.age && is_student == other.is_student;
        }
    };

    std::vector<Person> input = {
        {"Alice", 20, true},
        {"Bob", 25, false},
        {"Charlie", 22, true},
        {"David", 30, false}
    };
    
    auto result = AsDataFlow(input) 
        | Filter([](const Person& person) { 
            return person.age < 25 && person.is_student; 
        }) 
        | AsVector();
    
    ASSERT_EQ(result.size(), 2);
    ASSERT_THAT(result, testing::ElementsAre(
        Person{"Alice", 20, true},
        Person{"Charlie", 22, true}
    ));
}

TEST(FilterTest, FilterFilesByExtension) {
    std::vector<std::filesystem::path> input = {
        "file1.txt",
        "file2.cpp",
        "file3.h",
        "file4.txt",
        "file5.cpp"
    };
    
    auto result = AsDataFlow(input)
        | Filter([](const std::filesystem::path& path) { return path.extension() == ".cpp"; })
        | AsVector();
    
    ASSERT_THAT(result, testing::ElementsAre(
        std::filesystem::path("file2.cpp"),
        std::filesystem::path("file5.cpp")
    ));
}

TEST(FilterTest, CompositeFilters) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto isEven = [](int x) { return x % 2 == 0; };
    auto isGreaterThan5 = [](int x) { return x > 5; };
    
    auto result1 = AsDataFlow(input)
        | Filter(isEven)
        | Filter(isGreaterThan5)
        | AsVector();
    
    ASSERT_THAT(result1, testing::ElementsAre(6, 8, 10));
    
    auto combinedPredicate = [&](int x) { return isEven(x) && isGreaterThan5(x); };
    auto result2 = AsDataFlow(input)
        | Filter(combinedPredicate)
        | AsVector();
    
    ASSERT_THAT(result2, testing::ElementsAre(6, 8, 10));
    
    ASSERT_EQ(result1, result2);
}

TEST(FilterTest, FilterWithState) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8};
    
    int sum = 0;
    auto result = AsDataFlow(input)
        | Filter([&sum](int x) { 
            sum += x;
            return sum < 10;
        })
        | AsVector();
    
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3));
}