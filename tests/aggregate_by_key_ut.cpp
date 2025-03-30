#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

struct Employee {
    uint64_t department_id;
    std::string name;

    bool operator==(const Employee& other) const = default;
};

TEST(AggregateByKeyTest, CountingAggregatedValues) {
    std::vector<std::string> input = {"name4", "name0", "name1", "name0", "name2", "name0", "name1"};

    auto result =
        AsDataFlow(input)
            | AggregateByKey(
                std::size_t{0},
                [](const std::string&, std::size_t& accumulated) { ++accumulated; },
                [](const std::string& token) { return token; }
            )
            | AsVector();

    ASSERT_THAT(
        result,
        ::testing::ElementsAre(
            std::make_pair("name4", 1),
            std::make_pair("name0", 3),
            std::make_pair("name1", 2),
            std::make_pair("name2", 1)
        )
    );
}

TEST(AggregateByKeyTest, AggregatingWithSeveralOutputsForEachKey) {
    std::vector<Employee> employees = {
        {3, "name1"},
        {1, "name1"},
        {1, "name2"},
        {2, "name1"},
        {2, "name2"},
        {2, "name3"}
    };

    auto result =
        AsDataFlow(employees)
            | AggregateByKey(
                std::vector<Employee>{},
                [](const Employee& employee, std::vector<Employee>& accumulated) {
                    if (accumulated.size() == 2) {
                        return;
                    }
                    accumulated.push_back(employee);
                },
                [](const Employee& employee) { return employee.department_id; }
            )
            | AsVector();

    ASSERT_THAT(
        result,
        ::testing::ElementsAre(
            std::make_pair(3, std::vector<Employee>{Employee{3, "name1"}}),
            std::make_pair(1, std::vector<Employee>{Employee{1, "name1"}, Employee{1, "name2"}}),
            std::make_pair(2, std::vector<Employee>{Employee{2, "name1"}, Employee{2, "name2"}})
        )
    );
}

TEST(AggregateByKeyTest, EmptyInput) {
    std::vector<int> input;
    auto result =
        AsDataFlow(input)
        | AggregateByKey(
            0,
            [](int, int& acc) { ++acc; },
            [](int x) { return x; }
        )
        | AsVector();

    ASSERT_TRUE(result.empty());
}

TEST(AggregateByKeyTest, SingleElement) {
    std::vector<int> input = {42};
    auto result =
        AsDataFlow(input)
        | AggregateByKey(
            0,
            [](int, int& acc) { ++acc; },
            [](int x) { return x; }
        )
        | AsVector();

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0].first, 42);
    ASSERT_EQ(result[0].second, 1);
}

TEST(AggregateByKeyTest, SummationByKey) {
    std::vector<std::pair<int, int>> input = {
        {1, 5}, {2, 10}, {1, -3}, {2, 2}, {3, 100}
    };
    auto result =
        AsDataFlow(input)
        | AggregateByKey(
            0,
            [](const std::pair<int, int>& p, int& acc) { acc += p.second; },
            [](const std::pair<int, int>& p) { return p.first; }
        )
        | AsVector();

    ASSERT_THAT(
        result,
        ::testing::UnorderedElementsAre(
            std::make_pair(1, 2),
            std::make_pair(2, 12),
            std::make_pair(3, 100)
        )
    );
}

TEST(AggregateByKeyTest, MergeStrings) {
    std::vector<std::pair<std::string, std::string>> input = {
        {"A", "xy"}, {"B", "1"}, {"A", "z"}, {"B", "2"}, {"C", "!!!"}
    };
    auto result =
        AsDataFlow(input)
        | AggregateByKey(
            std::string{},
            [](const std::pair<std::string, std::string>& kvp, std::string& acc) {
                acc += kvp.second;
            },
            [](const std::pair<std::string, std::string>& kvp) {
                return kvp.first;
            }
        )
        | AsVector();

    ASSERT_THAT(
        result,
        ::testing::UnorderedElementsAre(
            std::make_pair(std::string{"A"}, std::string{"xyz"}),
            std::make_pair(std::string{"B"}, std::string{"12"}),
            std::make_pair(std::string{"C"}, std::string{"!!!"})
        )
    );
}