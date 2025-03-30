#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

struct Student {
  uint64_t group_id;
  std::string name;

  bool operator==(const Student &other) const = default;
};

struct Group {
  uint64_t id;
  std::string name;

  bool operator==(const Group &other) const = default;
};

struct Course {
  uint64_t id;
  std::string title;

  bool operator==(const Course &other) const = default;
};

struct Enrollment {
  uint64_t student_id;
  uint64_t course_id;

  bool operator==(const Enrollment &other) const = default;
};

TEST(JoinTest, JoinKV) {
  std::vector<KV<int, std::string>> left = {
      {0, "a"}, {1, "b"}, {2, "c"}, {3, "d"}, {1, "e"}};
  std::vector<KV<int, std::string>> right = {{0, "f"}, {1, "g"}, {3, "i"}};

  auto left_flow = AsDataFlow(left);
  auto right_flow = AsDataFlow(right);
  auto result = left_flow | Join(right_flow) | AsVector();

  ASSERT_THAT(result,
              testing::ElementsAre(
                  JoinResult<std::string, std::string>{"a", "f"},
                  JoinResult<std::string, std::string>{"b", "g"},
                  JoinResult<std::string, std::string>{"c", std::nullopt},
                  JoinResult<std::string, std::string>{"d", "i"},
                  JoinResult<std::string, std::string>{"e", "g"}));
}

TEST(JoinTest, JoinComparators) {
  std::vector<Student> students = {
      {0, "a"}, {1, "b"}, {2, "c"}, {3, "d"}, {1, "e"}};
  std::vector<Group> groups = {{0, "f"}, {1, "g"}, {3, "i"}};

  auto students_flow = AsDataFlow(students);
  auto groups_flow = AsDataFlow(groups);

  auto result =
      students_flow |
      Join(
          groups_flow, [](const Student &student) { return student.group_id; },
          [](const Group &group) { return group.id; }) |
      AsVector();

  ASSERT_THAT(result,
              testing::ElementsAre(
                  JoinResult<Student, Group>{Student{0, "a"}, Group{0, "f"}},
                  JoinResult<Student, Group>{Student{1, "b"}, Group{1, "g"}},
                  JoinResult<Student, Group>{Student{2, "c"}, std::nullopt},
                  JoinResult<Student, Group>{Student{3, "d"}, Group{3, "i"}},
                  JoinResult<Student, Group>{Student{1, "e"}, Group{1, "g"}}));
}

TEST(JoinTest, EmptyCollections) {

  std::vector<KV<int, std::string>> empty_left;
  std::vector<KV<int, std::string>> right = {{0, "f"}, {1, "g"}};

  auto result1 = AsDataFlow(empty_left) | Join(AsDataFlow(right)) | AsVector();
  ASSERT_TRUE(result1.empty());

  std::vector<KV<int, std::string>> left = {{0, "a"}, {1, "b"}};
  std::vector<KV<int, std::string>> empty_right;

  auto result2 = AsDataFlow(left) | Join(AsDataFlow(empty_right)) | AsVector();
  ASSERT_THAT(result2,
              testing::ElementsAre(
                  JoinResult<std::string, std::string>{"a", std::nullopt},
                  JoinResult<std::string, std::string>{"b", std::nullopt}));

  auto result3 =
      AsDataFlow(empty_left) | Join(AsDataFlow(empty_right)) | AsVector();
  ASSERT_TRUE(result3.empty());
}

TEST(JoinTest, NoMatchingKeys) {
  std::vector<KV<int, std::string>> left = {{1, "a"}, {2, "b"}, {3, "c"}};
  std::vector<KV<int, std::string>> right = {{4, "d"}, {5, "e"}, {6, "f"}};

  auto result = AsDataFlow(left) | Join(AsDataFlow(right)) | AsVector();

  ASSERT_THAT(result,
              testing::ElementsAre(
                  JoinResult<std::string, std::string>{"a", std::nullopt},
                  JoinResult<std::string, std::string>{"b", std::nullopt},
                  JoinResult<std::string, std::string>{"c", std::nullopt}));
}

TEST(JoinTest, MultiJoin) {

  std::vector<Student> students = {{1, "Gosha"}, {2, "Bob"}, {3, "Roma"}};

  std::vector<Group> groups = {{1, "Group A"}, {2, "Group B"}, {4, "Group D"}};

  std::vector<Course> courses = {
      {101, "Math"}, {102, "Physics"}, {103, "Chemistry"}};

  std::vector<Enrollment> enrollments = {
      {1, 101}, {1, 102}, {2, 101}, {3, 103}};

  auto students_with_groups =
      AsDataFlow(students) |
      Join(
          AsDataFlow(groups), [](const Student &s) { return s.group_id; },
          [](const Group &g) { return g.id; }) |
      AsVector();

  ASSERT_EQ(students_with_groups.size(), 3);
  ASSERT_TRUE(students_with_groups[0].right.has_value());
  ASSERT_TRUE(students_with_groups[1].right.has_value());
  ASSERT_FALSE(students_with_groups[2].right.has_value());

  std::vector<JoinResult<Student, std::vector<Enrollment>>> student_enrollments;

  for (const auto &student : students) {
    std::vector<Enrollment> student_courses;
    for (const auto &enrollment : enrollments) {
      if (enrollment.student_id == student.group_id) {
        student_courses.push_back(enrollment);
      }
    }

    if (!student_courses.empty()) {
      student_enrollments.push_back({student, student_courses});
    } else {
      student_enrollments.push_back({student, std::nullopt});
    }
  }

  ASSERT_EQ(student_enrollments.size(), 3);
  ASSERT_TRUE(student_enrollments[0].right.has_value());
  ASSERT_EQ(student_enrollments[0].right.value().size(), 2);
  ASSERT_TRUE(student_enrollments[1].right.has_value());
  ASSERT_EQ(student_enrollments[1].right.value().size(), 1);
  ASSERT_TRUE(student_enrollments[2].right.has_value());
  ASSERT_EQ(student_enrollments[2].right.value().size(), 1);
}

TEST(JoinTest, ResultOrdering) {
  std::vector<KV<int, std::string>> left = {{3, "c"}, {1, "a"}, {2, "b"}};
  std::vector<KV<int, std::string>> right = {{1, "x"}, {2, "y"}, {3, "z"}};

  auto result = AsDataFlow(left) | Join(AsDataFlow(right)) | AsVector();

  ASSERT_EQ(result.size(), 3);
  ASSERT_EQ(result[0].left, "c");
  ASSERT_EQ(result[1].left, "a");
  ASSERT_EQ(result[2].left, "b");
}