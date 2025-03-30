#pragma once

#include <type_traits>
#include <utility>
#include <vector>


template <typename Range> 
auto AsVectorImpl(Range&& range) {
  using value_type = typename std::decay_t<Range>::iterator::value_type;
  std::vector<value_type> result;
  for (const auto& elem: range) {
    result.push_back(elem);
  }

  return result;
}

class AsVectorOperation {
public:
  template <typename Range> 
  auto operator|(Range&& range) const {
    return AsVectorImpl(std::forward<Range>(range));
  }
};

inline auto AsVector() { return AsVectorOperation(); }

template <typename Range>
auto operator|(Range &&range, const AsVectorOperation& op) {
  return op.operator|(std::forward<Range>(range));
}