#pragma once

#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>


template <typename K, typename V> 
struct KV {
  K key;
  V value;
};

template <typename LeftType, typename RightType> 
struct JoinResult {
  LeftType left;
  std::optional<RightType> right;

  bool operator==(const JoinResult& other) const = default;
};

template <typename T> 
struct KeyExtractor {
  auto operator()(const T& kv) const { return kv.key; }
};

template <typename T> 
struct ValueExtractor {
  auto operator()(const T& kv) const { return kv.value; }
};

template <typename T> 
struct IdentityExtractor {
  template <typename U> 
  auto operator()(U&& item) const -> decltype(auto) {
    return std::forward<U>(item);
  }
};

template <typename LeftRange, typename RightRange, typename KeyExtractorLeft,
          typename KeyExtractorRight, typename ValueExtractorLeft,
          typename ValueExtractorRight>
class JoinIterator {
public:
  using key_type = std::decay_t<decltype(std::declval<KeyExtractorLeft>()(
      *std::declval<typename LeftRange::iterator>()))>;

  using left_value_type =
      std::decay_t<decltype(std::declval<ValueExtractorLeft>()(
          *std::declval<typename LeftRange::iterator>()))>;

  using right_value_type =
      std::decay_t<decltype(std::declval<ValueExtractorRight>()(
          *std::declval<typename RightRange::iterator>()))>;

  using iterator_category = std::input_iterator_tag;
  using value_type = JoinResult<left_value_type, right_value_type>;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type *;
  using reference = const value_type &;

  JoinIterator(typename LeftRange::iterator left_it,
               typename LeftRange::iterator left_end,
               const std::unordered_map<key_type, std::vector<right_value_type>>& right_index,
               KeyExtractorLeft key_extractor_left,
               ValueExtractorLeft value_extractor_left)
      : left_it_(left_it), left_end_(left_end), right_index_(right_index),
        key_extractor_left_(key_extractor_left),
        value_extractor_left_(value_extractor_left), right_values_(),
        right_index_pos_(0) {
    if (left_it_ != left_end_) {
      updateRightValues();
    }
  }

  JoinIterator() = default;

  reference operator*() const { return current_; }

  JoinIterator &operator++() {
    if (right_index_pos_ + 1 < right_values_.size()) {
      right_index_pos_++;
      current_ = JoinResult<left_value_type, right_value_type>{
          value_extractor_left_(*left_it_), right_values_[right_index_pos_]};
    } else {
      ++left_it_;
      right_index_pos_ = 0;
      if (left_it_ != left_end_) {
        updateRightValues();
      }
    }
    return *this;
  }

  bool operator!=(const JoinIterator &other) const {
    return left_it_ != other.left_it_ ||
           (left_it_ != left_end_ && other.left_it_ != other.left_end_ &&
            right_index_pos_ != other.right_index_pos_);
  }

private:
  typename LeftRange::iterator left_it_;
  typename LeftRange::iterator left_end_;
  const std::unordered_map<key_type, std::vector<right_value_type>>& right_index_;
  KeyExtractorLeft key_extractor_left_;
  ValueExtractorLeft value_extractor_left_;
  std::vector<right_value_type> right_values_;
  size_t right_index_pos_;
  value_type current_;

  void updateRightValues() {
    auto key = key_extractor_left_(*left_it_);
    auto it = right_index_.find(key);
    if (it != right_index_.end()) {
      right_values_ = it->second;
      if (!right_values_.empty()) {
        current_ = JoinResult<left_value_type, right_value_type>{
            value_extractor_left_(*left_it_), right_values_[right_index_pos_]};
      } else {
        current_ = JoinResult<left_value_type, right_value_type>{
            value_extractor_left_(*left_it_), std::nullopt};
      }
    } else {
      right_values_.clear();
      current_ = JoinResult<left_value_type, right_value_type>{
          value_extractor_left_(*left_it_), std::nullopt};
    }
  }
};

template <typename LeftRange, typename RightRange, typename KeyExtractorLeft,
          typename KeyExtractorRight, typename ValueExtractorLeft,
          typename ValueExtractorRight>
class JoinRange {
public:
  using iterator =
      JoinIterator<LeftRange, RightRange, KeyExtractorLeft, KeyExtractorRight,
                   ValueExtractorLeft, ValueExtractorRight>;
  using key_type = typename iterator::key_type;
  using value_type = typename iterator::value_type;
  using left_value_type = typename iterator::left_value_type;
  using right_value_type = typename iterator::right_value_type;

  JoinRange(LeftRange left_range, RightRange right_range,
            KeyExtractorLeft key_extractor_left,
            KeyExtractorRight key_extractor_right,
            ValueExtractorLeft value_extractor_left,
            ValueExtractorRight value_extractor_right)
      : left_range_(std::move(left_range)),
        key_extractor_left_(std::move(key_extractor_left)),
        value_extractor_left_(std::move(value_extractor_left)), right_index_() {
    for (const auto& item : right_range) {
      right_index_[key_extractor_right(item)].push_back(
          value_extractor_right(item));
    }
  }

  iterator begin() {
    return iterator(left_range_.begin(), left_range_.end(), right_index_,
                    key_extractor_left_, value_extractor_left_);
  }

  iterator end() {
    return iterator(left_range_.end(), left_range_.end(), right_index_,
                    key_extractor_left_, value_extractor_left_);
  }

private:
  LeftRange left_range_;
  KeyExtractorLeft key_extractor_left_;
  ValueExtractorLeft value_extractor_left_;
  std::unordered_map<key_type, std::vector<right_value_type>> right_index_;
};

template <typename RightRange,
          typename KeyExtractorLeft =
              KeyExtractor<typename std::decay_t<RightRange>::value_type>,
          typename KeyExtractorRight =
              KeyExtractor<typename std::decay_t<RightRange>::value_type>,
          typename ValueExtractorLeft =
              ValueExtractor<typename std::decay_t<RightRange>::value_type>,
          typename ValueExtractorRight =
              ValueExtractor<typename std::decay_t<RightRange>::value_type>>
class JoinAdapter {
public:
  JoinAdapter(
      RightRange right_range,
      KeyExtractorLeft key_extractor_left =
          KeyExtractor<typename std::decay_t<RightRange>::value_type>{},
      KeyExtractorRight key_extractor_right =
          KeyExtractor<typename std::decay_t<RightRange>::value_type>{},
      ValueExtractorLeft value_extractor_left =
          ValueExtractor<typename std::decay_t<RightRange>::value_type>{},
      ValueExtractorRight value_extractor_right =
          ValueExtractor<typename std::decay_t<RightRange>::value_type>{})
      : right_range_(std::move(right_range)),
        key_extractor_left_(std::move(key_extractor_left)),
        key_extractor_right_(std::move(key_extractor_right)),
        value_extractor_left_(std::move(value_extractor_left)),
        value_extractor_right_(std::move(value_extractor_right)) {}

  template <typename LeftRange> auto operator()(LeftRange&& left_range) const {
    return JoinRange<std::decay_t<LeftRange>, RightRange, KeyExtractorLeft,
                     KeyExtractorRight, ValueExtractorLeft,
                     ValueExtractorRight>(
        std::forward<LeftRange>(left_range), right_range_, key_extractor_left_,
        key_extractor_right_, value_extractor_left_, value_extractor_right_);
  }

private:
  RightRange right_range_;
  KeyExtractorLeft key_extractor_left_;
  KeyExtractorRight key_extractor_right_;
  ValueExtractorLeft value_extractor_left_;
  ValueExtractorRight value_extractor_right_;
};

template <typename RightRange> auto Join(RightRange&& right_range) {
  return JoinAdapter<std::decay_t<RightRange>>(
      std::forward<RightRange>(right_range));
}

template <typename RightRange, typename KeyExtractorLeft,
          typename KeyExtractorRight>
auto Join(RightRange&& right_range, KeyExtractorLeft key_extractor_left,
          KeyExtractorRight key_extractor_right) {
  return JoinAdapter<
      std::decay_t<RightRange>, KeyExtractorLeft, KeyExtractorRight,
      IdentityExtractor<typename std::decay_t<RightRange>::value_type>,
      IdentityExtractor<typename std::decay_t<RightRange>::value_type>>(
      std::forward<RightRange>(right_range), std::move(key_extractor_left),
      std::move(key_extractor_right),
      IdentityExtractor<typename std::decay_t<RightRange>::value_type>{},
      IdentityExtractor<typename std::decay_t<RightRange>::value_type>{});
}

template <typename LeftRange, typename RightRange, typename KeyExtractorLeft,
          typename KeyExtractorRight, typename ValueExtractorLeft,
          typename ValueExtractorRight>
auto operator|(
    LeftRange&& left_range,
    const JoinAdapter<RightRange, KeyExtractorLeft, KeyExtractorRight,
                      ValueExtractorLeft, ValueExtractorRight>& join_adapter) {
  return join_adapter(std::forward<LeftRange>(left_range));
}