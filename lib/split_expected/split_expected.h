#pragma once

#include <expected>
#include <functional>
#include <utility>

template <typename Parser,typename Iterator>
class IteratorExpected {
public:
  using iterator_category = std::input_iterator_tag;
  using function_traits = decltype(std::function{std::declval<Parser>()});
  using expected_type = typename function_traits::result_type;
  using value_type = typename expected_type::value_type;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  IteratorExpected(Iterator begin, Iterator end): 
  current_(begin), end_it_(end) {
    SkipUnexpected();
  }

  auto operator*() const {
    return (*current_).value();
  }

  IteratorExpected& operator++() {
    ++current_;
    SkipUnexpected();
    return *this;
  }

  bool operator!=(const IteratorExpected& other) const {
    return current_ != other.current_;
  }

private:

  void SkipUnexpected() {
    while (current_ != end_it_ && !(*current_).has_value()) {
      ++current_;
    }
  }

  Iterator current_;
  Iterator end_it_;
};

template <typename Parser,typename Iterator>
class IteratorUnexpected {
public:
  using iterator_category = std::input_iterator_tag;
  using function_traits = decltype(std::function{std::declval<Parser>()});
  using expected_type = typename function_traits::result_type;
  using value_type = typename expected_type::error_type;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  IteratorUnexpected(Iterator begin, Iterator end): 
  current_(begin), end_it_(end) {
    SkipExpected();
  }

  auto operator*() const {
    return (*current_).error();
  }

  IteratorUnexpected& operator++() {
    ++current_;
    SkipExpected();
    return *this;
  }

  bool operator!=(const IteratorUnexpected& other) const {
    return current_ != other.current_;
  }

private:

  void SkipExpected() {
    while (current_ != end_it_ && (*current_).has_value()) {
      ++current_;
    }
  }
  
  Iterator current_;
  Iterator end_it_;
};

template<typename Parser, typename Range>
class RangeExpected {
public:
  using range_iterator = decltype(std::declval<Range>().begin());;
  using iterator = IteratorExpected<Parser, range_iterator>;
  
  explicit RangeExpected(Range range) : range_(range) {}
  
  iterator begin() { 
    return iterator(range_.begin(), range_.end()); 
  }
  
  iterator end() { 
    return iterator(range_.end(), range_.end()); 
  }
  
private:
  Range range_;
};

template<typename Parser, typename Range>
class RangeUnexpected {
public:
  using range_iterator = decltype(std::declval<Range>().begin());;
  using iterator = IteratorUnexpected<Parser, range_iterator>;
  
  explicit RangeUnexpected(Range range) : range_(range) {}
  
  iterator begin() { 
    return iterator(range_.begin(), range_.end()); 
  }
  
  iterator end() { 
    return iterator(range_.end(), range_.end()); 
  }
  
private:
  Range range_;
};

template <typename Parser>
class SplitExpectedAdapter {
public:
  explicit SplitExpectedAdapter(Parser parser) : 
  parser_(std::move(parser)) { }

  template <typename Range>
  auto operator|(Range&& range) const {
    return GetSplitRanges();
  }
  
  template <typename Range>
  auto GetSplitRanges(Range range) const {
    return std::make_pair(RangeUnexpected<Parser, Range>(range),
                          RangeExpected<Parser, Range>(range));
  }
  private:

  Parser parser_;
};

template<typename Parser>
auto SplitExpected(Parser parser_func) {
  return SplitExpectedAdapter<Parser>(parser_func);
}

template<typename Range, typename Parser>
auto operator|(Range&& range, const SplitExpectedAdapter<Parser>& split) {
  return split.GetSplitRanges(range);
}