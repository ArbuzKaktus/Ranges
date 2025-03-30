#pragma once

#include <utility>
#include <fstream>
#include <string>

template <typename Range, typename OutputStream> 
class OutProxy {
public:
  OutProxy(Range&& range, OutputStream& stream)
      : range_(std::forward<Range>(range)), stream_(stream) {
    for (const auto& item : range_) {
      outputItem(item);
    }
  }

  auto begin() { return nullptr; }
  auto end() { return nullptr; }
  auto begin() const { return nullptr; }
  auto end() const { return nullptr; }

private:
  Range range_;
  OutputStream& stream_;
  
  template <typename T>
  void outputItem(T& item) {
    if constexpr (std::is_same_v<std::decay_t<T>, std::ifstream>) {
      auto &file = const_cast<std::decay_t<decltype(item)> &>(item);

      std::string content;
      content.assign(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());

      stream_ << content;
    } else {
      stream_ << item;
    }
  }
};

template <typename OutputStream> class OutOperation {
public:
  explicit OutOperation(OutputStream& stream) : stream_(stream) {}

  template <typename Range> 
  auto operator|(Range&& range) const {
    return OutProxy<Range, OutputStream>(std::forward<Range>(range), stream_);
  }

private:
  OutputStream &stream_;
};

template <typename OutputStream> 
auto Out(OutputStream& stream) {
  return OutOperation<OutputStream>(stream);
}

template <typename Range, typename OutputStream>
auto operator|(Range&& range, const OutOperation<OutputStream>& op) {
  return op.operator|(std::forward<Range>(range));
}