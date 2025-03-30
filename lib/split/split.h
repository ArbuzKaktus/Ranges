#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


template <typename Iterator> 
class SplitIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = std::string;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;

  SplitIterator(Iterator current, Iterator end, const std::string& delimiters)
      : current_(current), end_(end), delimiters_(delimiters),
        current_index_(0) {
    if (current_ != end_) {
      splitCurrentString();
    }
  }

  SplitIterator(Iterator end) : current_(end), end_(end), current_index_(0) {}

  SplitIterator() : current_(Iterator{}), end_(Iterator{}), current_index_(0) {}

  reference operator*() const { return tokens_[current_index_]; }

  pointer operator->() const { return &tokens_[current_index_]; }

  SplitIterator &operator++() {
    current_index_++;

    if (current_index_ >= tokens_.size()) {
      ++current_;
      current_index_ = 0;

      if (current_ != end_) {
        splitCurrentString();
      }
    }

    return *this;
  }

  bool operator!=(const SplitIterator& other) const {
    return current_ != other.current_ ||
           (current_ != end_ && current_index_ != other.current_index_);
  }

private:
  Iterator current_;
  Iterator end_;
  std::string delimiters_;
  mutable std::vector<std::string> tokens_;
  size_t current_index_;

  void splitCurrentString() const {
    tokens_.clear();
    std::string input = getCurrentContent();
  if (delimiters_.empty()) {
    for (char c : input) {
      tokens_.push_back(std::string(1, c));
    }
    return;
  }
    std::string localDelimiters = delimiters_;
    if (delimiters_.find('\n') == std::string::npos) {
      localDelimiters += '\n';
    }

    size_t prev = 0;
    size_t pos = 0;

    while ((pos = input.find_first_of(localDelimiters, prev)) != std::string::npos) {
      tokens_.push_back(input.substr(prev, pos - prev));
      prev = pos + 1;
    }

    tokens_.push_back(input.substr(prev));
  }

  std::string getCurrentContent() const {
    using CurrentType = std::decay_t<decltype(*std::declval<Iterator>())>;

    if constexpr (std::is_same_v<CurrentType, std::ifstream>) {
      return readFromIfstream(current_);
    } else if constexpr (std::is_same_v<CurrentType, std::stringstream> ||
                         std::is_same_v<CurrentType,
                                        std::basic_stringstream<char>> ||
                         requires(decltype(*std::declval<Iterator>()) s) {
                           s.str();
                         }) {
      return (*current_).str();
    } else if constexpr (std::is_convertible_v<CurrentType, std::string>) {
      return static_cast<std::string>(*current_);
    } else {
      std::stringstream ss;
      ss << *current_;
      return ss.str();
    }
  }

  std::string readFromIfstream(const Iterator& iter) const {
    Iterator temp = iter;
    auto &file = *temp;

    std::string content;

    std::streampos pos = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);

    std::stringstream ss;
    ss << file.rdbuf();
    content = ss.str();

    file.clear();
    file.seekg(pos);

    return content;
  }
};

template <typename Range> 
class SplitRange {
public:
  using value_type = std::string;
  using iterator = SplitIterator<typename std::decay_t<Range>::iterator>;
  using const_iterator =
      SplitIterator<typename std::decay_t<Range>::const_iterator>;

  SplitRange(Range&& range, const std::string& delimiters)
      : range_(std::forward<Range>(range)), delimiters_(delimiters) {}

  auto begin() { return iterator(range_.begin(), range_.end(), delimiters_); }

  auto end() { return iterator(range_.end()); }

  auto begin() const {
    return const_iterator(range_.begin(), range_.end(), delimiters_);
  }

  auto end() const { return const_iterator(range_.end()); }

private:
  Range range_;
  std::string delimiters_;
};

class SplitOperation {
public:
  explicit SplitOperation(const std::string& delimiters)
      : delimiters_(delimiters) {}

  template <typename Range> 
  auto operator|(Range&& range) const {
    return SplitRange<Range>(std::forward<Range>(range), delimiters_);
  }

private:
  std::string delimiters_;
};

inline auto Split(const std::string& delimiters) {
  return SplitOperation(delimiters);
}

template <typename Range>
auto operator|(Range&& range, const SplitOperation& op) {
  return op | std::forward<Range>(range);
}