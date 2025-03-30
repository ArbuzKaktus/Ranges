#pragma once

#include <type_traits> 

template <typename Container>
class AsDataFlowRange {
public:
  using value_type = typename std::decay_t<Container>::value_type;
  using iterator = typename std::decay_t<Container>::iterator;
  using const_iterator = typename std::decay_t<Container>::const_iterator;

  AsDataFlowRange(Container& cont): 
  begin_it_(cont.begin()), end_it_(cont.end()), 
  const_begin_it_(cont.begin()), const_end_it_(cont.end()) { }

  const_iterator begin() const { return const_begin_it_; }
  const_iterator end() const { return const_end_it_; }
  iterator begin() { return begin_it_; }
  iterator end() { return end_it_; }

private:
  iterator begin_it_;
  iterator end_it_;
  const_iterator const_begin_it_;
  const_iterator const_end_it_;
};

template <typename Container>
auto AsDataFlow(Container& cont) {
  return AsDataFlowRange<Container>(cont);
}