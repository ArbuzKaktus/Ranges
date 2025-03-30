#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

template <typename Iterator> 
class DropNulloptIterator {
public:
  using iterator_category = 
      std::input_iterator_tag;
  using value_type =
      typename std::iterator_traits<Iterator>::value_type::value_type;
  using difference_type =
      typename std::iterator_traits<Iterator>::difference_type;
  using pointer = value_type*;
  using reference = value_type&;

  DropNulloptIterator(Iterator current, Iterator end)
      : current_(current), end_(end) {
    skipNullopt();
  }

  reference operator*() const { return current_->value(); }

  DropNulloptIterator &operator++() {
    ++current_;
    skipNullopt();
    return *this;
  }

  bool operator!=(const DropNulloptIterator& other) const {
    return current_ != other.current_;
  }

private:
  Iterator current_;
  Iterator end_;

  void skipNullopt() {
    while (current_ != end_ && !current_->has_value()) {
      ++current_;
    }
  }
};

template <typename Range> 
class DropNulloptRange {
public:
  using value_type = typename std::decay_t<Range>::value_type::value_type;
  using iterator = DropNulloptIterator<typename std::decay_t<Range>::iterator>;
  using const_iterator =
      DropNulloptIterator<typename std::decay_t<Range>::const_iterator>;

  explicit DropNulloptRange(Range&& range)
      : range_(std::forward<Range>(range)) {}

  auto begin() { return iterator(range_.begin(), range_.end()); }

  auto end() { return iterator(range_.end(), range_.end()); }

  auto begin() const { return const_iterator(range_.begin(), range_.end()); }

  auto end() const { return const_iterator(range_.end(), range_.end()); }

private:
  Range range_;
};

class DropNulloptOperation {
public:
  template <typename Range> 
  auto operator|(Range&& range) const {
    return DropNulloptRange<Range>(std::forward<Range>(range));
  }
};

inline auto DropNullopt() { return DropNulloptOperation(); }

template <typename Range>
auto operator|(Range&& range, const DropNulloptOperation& op) {
  return op.operator|(std::forward<Range>(range));
}