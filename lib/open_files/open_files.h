#pragma once

#include <fstream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>


template <typename Iterator> 
class OpenFilesIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = std::ifstream;
  using difference_type = std::ptrdiff_t;
  using pointer = std::shared_ptr<std::ifstream>;
  using reference = std::ifstream &;

  OpenFilesIterator(Iterator current, Iterator end)
      : current_(current), end_(end) {
    if (current_ != end_) {
      openCurrentFile();
    }
  }

  reference operator*() const { return *current_file_; }

  OpenFilesIterator &operator++() {
    ++current_;
    if (current_ != end_) {
      openCurrentFile();
    } else {
      current_file_.reset();
    }
    return *this;
  }

  bool operator!=(const OpenFilesIterator &other) const {
    return current_ != other.current_;
  }

private:
  Iterator current_;
  Iterator end_;
  pointer current_file_;

  void openCurrentFile() {
    current_file_ = std::make_shared<std::ifstream>(*current_);
    if (!*current_file_) {
      ++(*this);
    }
  }
};

template <typename Range> 
class OpenFilesRange {
public:
  using iterator = OpenFilesIterator<typename std::decay_t<Range>::iterator>;
  using const_iterator =
      OpenFilesIterator<typename std::decay_t<Range>::const_iterator>;
  using value_type = typename iterator::value_type;

  explicit OpenFilesRange(Range&& range) : range_(std::forward<Range>(range)) {}

  iterator begin() { return iterator(range_.begin(), range_.end()); }

  iterator end() { return iterator(range_.end(), range_.end()); }

  const_iterator begin() const {
    return const_iterator(range_.begin(), range_.end());
  }

  const_iterator end() const {
    return const_iterator(range_.end(), range_.end());
  }

private:
  Range range_;
};

class OpenFilesOperation {
public:
  OpenFilesOperation() = default;

  template <typename Range> 
  auto operator|(Range&& range) const {
    return OpenFilesRange<Range>(std::forward<Range>(range));
  }
};

inline auto OpenFiles() { return OpenFilesOperation(); }

template <typename Range>
auto operator|(Range&& range, const OpenFilesOperation& op) {
  return op.operator|(std::forward<Range>(range));
}