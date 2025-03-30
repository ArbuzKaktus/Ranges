#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <sstream>
#include <filesystem>
#include <utility>

TEST(DirTest, NonRecursive) {
    std::string path = 
    std::filesystem::current_path().parent_path().parent_path().string() + "\\tests\\fold_for_tests";
    std::stringstream out_stream;
    bool recursive = false;
    auto pip = 
    Dir(path, recursive)
    | Filter([](std::filesystem::path& p){ return p.extension() == ".txt"; })
    | OpenFiles()
    | Out(out_stream);
    ASSERT_EQ(out_stream.str(), "ya arbuzik\n");
}

TEST(DirTest, Recursive) {
    std::string path = 
    std::filesystem::current_path().parent_path().parent_path().string() + "\\tests\\fold_for_tests";
    std::stringstream out_stream;
    bool recursive = true;
    Dir(path, recursive)
    | Filter([](std::filesystem::path& p){ return p.extension() == ".txt"; })
    | OpenFiles()
    | Out(out_stream);
    ASSERT_EQ(out_stream.str(), "ya arbuzik\nim file in some fold lol\neto ochen kruto\n");
}

/*
Программа считает частоту слов, встречающихся во всех файлах в директории 
и выводит итоговую статистику в консоль
*/

TEST(DirTest, MainCpp) {
  std::string path = 
  std::filesystem::current_path().parent_path().parent_path().string() + "\\tests\\fold_for_tests";
  std::stringstream out_stream;
  bool recursive = true;
  auto result = Dir(path, recursive) 
    | Filter([](std::filesystem::path& p){ return p.extension() == ".txt"; })
    | OpenFiles()
    | Split("\n ,.;")
    | Transform(
        [](std::string& token) { 
            std::transform(token.begin(), token.end(), token.begin(), [](char c){return std::tolower(c);});
            return token;
        })
    | AggregateByKey(
        0uz, 
        [](const std::string&, size_t& count) { ++count;},
        [](const std::string& token) { return token;}
      );
      ASSERT_THAT(result, testing::ElementsAre(
        std::make_pair("ya", 1), 
        std::make_pair("file", 1),
        std::make_pair("im", 1),
        std::make_pair("arbuzik", 1),
        std::make_pair("", 3),
        std::make_pair("in", 1),
        std::make_pair("some", 1),
        std::make_pair("fold", 1),
        std::make_pair("lol", 1),
        std::make_pair("eto", 1),
        std::make_pair("kruto", 1),
        std::make_pair("ochen", 1)));
}