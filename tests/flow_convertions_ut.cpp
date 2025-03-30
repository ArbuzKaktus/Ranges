
#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <array>
#include <list>
#include <stack>
#include <string>
#include <deque>
#include <set>

TEST(FlowConvertionsTest, AsDataFlow) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto flow = AsDataFlow(input);
    ASSERT_THAT(flow, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, AsVector) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | AsVector();
    ASSERT_THAT(result, testing::ElementsAreArray(std::vector<int>{1, 2, 3, 4, 5}));
}


TEST(FlowConvertionsTest, EmptyContainer) {
    std::vector<double> input = {};
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(FlowConvertionsTest, AsDataFlowFromArray) {
    std::array<int, 5> input = {5, 4, 3, 2, 1};
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(5, 4, 3, 2, 1));
}

TEST(FlowConvertionsTest, AsDataFlowFromList) {
    std::list<std::string> input = {"apple", "banana", "cherry"};
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("apple", "banana", "cherry"));
    ASSERT_TRUE((std::is_same_v<decltype(result), std::vector<std::string>>));
}

TEST(FlowConvertionsTest, AsDataFlowFromDeque) {
    std::deque<char> input = {'a', 'b', 'c', 'd'};
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    ASSERT_THAT(result, testing::ElementsAre('a', 'b', 'c', 'd'));
}

TEST(FlowConvertionsTest, AsDataFlowFromSet) {
    std::set<int> input = {3, 1, 4, 5, 2};
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, AsDataFlowCustomType) {
    struct Person {
        std::string name;
        int age;
        
        bool operator==(const Person& other) const {
            return name == other.name && age == other.age;
        }
    };
    
    std::vector<Person> input = {
        {"Alice", 25},
        {"Bob", 30},
        {"Charlie", 35}
    };
    
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0].name, "Alice");
    ASSERT_EQ(result[0].age, 25);
    ASSERT_EQ(result[1].name, "Bob");
    ASSERT_EQ(result[1].age, 30);
    ASSERT_EQ(result[2].name, "Charlie");
    ASSERT_EQ(result[2].age, 35);
}

TEST(FlowConvertionsTest, AsDataFlowPairs) {
    std::vector<std::pair<std::string, int>> input = {
        {"key1", 1},
        {"key2", 2},
        {"key3", 3}
    };
    
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    
    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0].first, "key1");
    ASSERT_EQ(result[0].second, 1);
    ASSERT_EQ(result[2].first, "key3");
    ASSERT_EQ(result[2].second, 3);
}

TEST(FlowConvertionsTest, AsDataFlowNestedContainers) {
    std::vector<std::vector<int>> input = {
        {1, 2, 3},
        {4, 5},
        {6, 7, 8, 9}
    };
    
    auto flow = AsDataFlow(input);
    auto result = flow | AsVector();
    
    ASSERT_EQ(result.size(), 3);
    ASSERT_THAT(result[0], testing::ElementsAre(1, 2, 3));
    ASSERT_THAT(result[1], testing::ElementsAre(4, 5));
    ASSERT_THAT(result[2], testing::ElementsAre(6, 7, 8, 9));
}

TEST(FlowConvertionsTest, AsDataFlowRValue) {
    auto flow = AsDataFlow(std::vector<int>{10, 20, 30, 40});
    auto result = flow | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(10, 20, 30, 40));
}

TEST(FlowConvertionsTest, ModifyAfterConversion) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | AsVector();
    
    input[0] = 100;
    ASSERT_EQ(result[0], 1);
    
    result[1] = 200;
    ASSERT_EQ(input[1], 2);
}

TEST(FlowConvertionsTest, IntegrationWithOtherAdapters) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto result = AsDataFlow(input)
        | Filter([](int x) { return x % 2 == 0; })
        | Transform([](int x) { return x * x; })
        | AsVector();
    
    ASSERT_THAT(result, testing::ElementsAre(4, 16, 36, 64, 100));
}

TEST(FlowConvertionsTest, FlowWithoutAsDataFlow) {
    std::vector<int> input_vector = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::list<int> input_list = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::deque<int> input_deque = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result_vector = input_vector 
        | Filter([](int x) { return x % 2 == 0; })
        | Transform([](int x) { return x * x; })
        | AsVector();
    
    auto result_list = input_list
        | Filter([](int x) { return x % 2 == 0; })
        | Transform([](int x) { return x * x; })
        | AsVector();

    auto result_deque = input_deque 
        | Filter([](int x) { return x % 2 == 0; })
        | Transform([](int x) { return x * x; })
        | AsVector();

    ASSERT_THAT(result_vector, testing::ElementsAre(4, 16, 36, 64, 100));
    ASSERT_THAT(result_list, testing::ElementsAre(4, 16, 36, 64, 100));
    ASSERT_THAT(result_deque, testing::ElementsAre(4, 16, 36, 64, 100));
}