#pragma once

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>


template <typename Container> 
auto AsDataFlow(Container &&container) {
  return std::forward<Container>(container);
}

template <typename Range> 
auto AsVector(Range &&range) {
  using value_type = typename std::decay_t<Range>::value_type;
  return std::vector<value_type>(std::begin(range), std::end(range));
}

template <typename InitialValue, typename AggregateFunc,
          typename KeyExtractorFunc>
class AggregateByKeyRange {
public:
  AggregateByKeyRange(InitialValue initialValue, AggregateFunc aggregator,
                      KeyExtractorFunc keyExtractor)
      : initialValue_(std::move(initialValue)),
        aggregator_(std::move(aggregator)),
        keyExtractor_(std::move(keyExtractor)) {}

  template <typename Range> 
  auto operator|(Range&& range) const {
    using Element = typename std::decay_t<Range>::value_type;
    using Key = std::decay_t<decltype(keyExtractor_(std::declval<Element>()))>;
    using AccumulatedValue = InitialValue;

    std::unordered_map<Key, AccumulatedValue> aggregated;

    for (const auto &element : range) {
      Key key = keyExtractor_(element);

      if (aggregated.find(key) == aggregated.end()) {
        aggregated[key] = initialValue_;
      }

      aggregator_(element, aggregated[key]);
    }

    std::vector<std::pair<Key, AccumulatedValue>> result;
    result.reserve(aggregated.size());

    for (const auto &[key, value] : aggregated) {
      result.emplace_back(key, value);
    }

    return result;
  }

private:
  InitialValue initialValue_;
  AggregateFunc aggregator_;
  KeyExtractorFunc keyExtractor_;
};

template <typename InitialValue, typename AggregateFunc,
          typename KeyExtractorFunc>
auto AggregateByKey(InitialValue initialValue, AggregateFunc aggregator,
                    KeyExtractorFunc keyExtractor) {
  return AggregateByKeyRange<InitialValue, AggregateFunc, KeyExtractorFunc>(
      std::move(initialValue), std::move(aggregator), std::move(keyExtractor));
}

template <typename Range, typename InitialValue, typename AggregateFunc,
          typename KeyExtractorFunc>
auto operator|(Range&& range, const AggregateByKeyRange<InitialValue, AggregateFunc,
              KeyExtractorFunc>& operation) {
  return operation.operator|(std::forward<Range>(range));
}