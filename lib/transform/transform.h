#pragma once

#include <type_traits>
#include <utility>

template <typename Iterator, typename UnaryFunction>
class TransformIterator {
public:

  using iterator_category = typename Iterator::iterator_category;
  using difference_type = typename Iterator::difference_type;

  using value_type = std::decay_t<decltype(std::declval<UnaryFunction>()(*std::declval<Iterator>()))>;
  using pointer = value_type*;
  using reference = value_type&;

  TransformIterator(Iterator it, UnaryFunction func)
    : current_(it), func_(func) {}

  auto operator*() const {
    return func_(*current_);
  }

  TransformIterator& operator++() {
    ++current_;
    return *this;
  }

  bool operator!=(const TransformIterator& other) const {
    return current_ != other.current_;
  }

private:
  Iterator current_;
  UnaryFunction func_;
};

template <typename Range, typename UnaryFunction>
class TransformRange {
public:
  using iterator = TransformIterator<typename std::decay_t<Range>::iterator, UnaryFunction>;
  using const_iterator = TransformIterator<typename std::decay_t<Range>::const_iterator, UnaryFunction>;
  using value_type = typename iterator::value_type;

  TransformRange(Range&& range, UnaryFunction func)
    : range_(std::forward<Range>(range)), func_(func) {}

  iterator begin() {
    return iterator(range_.begin(), func_);
  }

  iterator end() {
    return iterator(range_.end(), func_);
  }

private:
  Range range_;
  UnaryFunction func_;
};

template <typename UnaryFunction>
class TransformOperation {
public:
  explicit TransformOperation(UnaryFunction func)
    : func_(std::move(func)) {}

  template <typename Range>
  auto operator|(Range&& range) const {
    return TransformRange<Range, UnaryFunction>(
        std::forward<Range>(range), func_);
  }

private:
  UnaryFunction func_;
};

template <typename UnaryFunction>
auto Transform(UnaryFunction func) {
  return TransformOperation<UnaryFunction>(std::move(func));
}

template <typename Range, typename UnaryFunction>
auto operator|(Range&& range, const TransformOperation<UnaryFunction>& op) {
  return op.operator|(std::forward<Range>(range));
}