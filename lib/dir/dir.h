#pragma once

#include <filesystem>
#include <variant>

class DirIterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::filesystem::path;
  using pointer = std::filesystem::path*;
  using reference = std::filesystem::path&;
  using iterator_category = std::input_iterator_tag;

  DirIterator(std::filesystem::directory_iterator it)
      : it_(it), is_recursive_(false) {}
  DirIterator(std::filesystem::recursive_directory_iterator it)
      : it_(it), is_recursive_(true) {}

  reference operator*() const {
    if (is_recursive_) {
      static std::filesystem::path path;
      path =
          std::get<std::filesystem::recursive_directory_iterator>(it_)->path();
      return path;
    } else {
      static std::filesystem::path path;
      path = std::get<std::filesystem::directory_iterator>(it_)->path();
      return path;
    }
  }

  DirIterator &operator++() {
    if (is_recursive_) {
      ++std::get<std::filesystem::recursive_directory_iterator>(it_);
    } else {
      ++std::get<std::filesystem::directory_iterator>(it_);
    }
    return *this;
  }

  bool operator!=(const DirIterator &other) const {
    if (is_recursive_ != other.is_recursive_) {
      return true;
    }

    if (is_recursive_) {
      return std::get<std::filesystem::recursive_directory_iterator>(it_) !=
             std::get<std::filesystem::recursive_directory_iterator>(other.it_);
    } else {
      return std::get<std::filesystem::directory_iterator>(it_) !=
             std::get<std::filesystem::directory_iterator>(other.it_);
    }
  }

private:
  std::variant<std::filesystem::directory_iterator,
               std::filesystem::recursive_directory_iterator>
      it_;
  bool is_recursive_;
};

class Dir {
public:
  using value_type = typename DirIterator::value_type;
  using iterator = DirIterator;
  using const_iterator = DirIterator;

  Dir(const std::filesystem::path &path, bool recursive = false)
      : path_(path), recursive_(recursive) {}

  iterator begin() const {
    if (recursive_) {
      return iterator(std::filesystem::recursive_directory_iterator(path_));
    } else {
      return iterator(std::filesystem::directory_iterator(path_));
    }
  }

  iterator end() const {
    if (recursive_) {
      return iterator(std::filesystem::recursive_directory_iterator());
    } else {
      return iterator(std::filesystem::directory_iterator());
    }
  }

private:
  std::filesystem::path path_;
  bool recursive_;
};