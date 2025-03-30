#pragma once

#include <iterator>
#include <type_traits>

template <typename Iterator, typename Predicate> 
class FilterIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using value_type = typename std::iterator_traits<Iterator>::value_type;
  using difference_type =
      typename std::iterator_traits<Iterator>::difference_type;
  using pointer = typename std::iterator_traits<Iterator>::pointer;
  using reference = typename std::iterator_traits<Iterator>::reference;

  FilterIterator(Iterator current, Iterator end, Predicate predicate)
      : current_(current), end_(end), predicate_(predicate) {
    findNext();
  }

  reference operator*() const { return *current_; }

  FilterIterator &operator++() {
    ++current_;
    findNext();
    return *this;
  }

  bool operator!=(const FilterIterator& other) const {
    return current_ != other.current_;
  }

private:
  Iterator current_;
  Iterator end_;
  Predicate predicate_;

  void findNext() {
    while (current_ != end_ && !predicate_(*current_)) {
      ++current_;
    }
  }
};

template <typename Range, typename Predicate> 
class FilterRange {
public:
  using iterator =
      FilterIterator<typename std::decay_t<Range>::iterator, Predicate>;
  using const_iterator = typename std::decay_t<Range>::const_iterator;
  using value_type = typename std::decay_t<Range>::value_type;

  FilterRange(Range&& range, Predicate predicate)
      : range_(std::forward<Range>(range)), predicate_(std::move(predicate)) {}

  iterator begin() {
    return iterator(range_.begin(), range_.end(), predicate_);
  }

  iterator end() { return iterator(range_.end(), range_.end(), predicate_); }

  const_iterator begin() const {
    return const_iterator(range_.begin(), range_.end(), predicate_);
  }

  const_iterator end() const {
    return const_iterator(range_.end(), range_.end(), predicate_);
  }

private:
  Range range_;
  Predicate predicate_;
};

template <typename Predicate> 
class FilterOperation {
public:
  explicit FilterOperation(Predicate predicate)
      : predicate_(std::move(predicate)) {}

  template <typename Range> auto operator|(Range&& range) const {
    return FilterRange<Range, Predicate>(std::forward<Range>(range),
                                         predicate_);
  }

private:
  Predicate predicate_;
};

template <typename Predicate> 
auto Filter(Predicate predicate) {
  return FilterOperation<Predicate>(std::move(predicate));
}

template <typename Range, typename Predicate>
auto operator|(Range&& range, const FilterOperation<Predicate>& filter) {
  return filter.operator|(std::forward<Range>(range));
}