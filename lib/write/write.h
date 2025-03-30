#pragma once

#include <fstream>
#include <string>
#include <utility>


template <typename Range, typename OutputStream> class WriteProxy {
public:
  WriteProxy(Range &&range, OutputStream &stream, char delimiter)
      : range_(std::forward<Range>(range)), stream_(stream),
        delimiter_(delimiter) {
    for (const auto &item : range_) {
      if constexpr (std::is_same_v<std::decay_t<decltype(item)>,
                                   std::ifstream> ||
                    std::is_base_of_v<std::ifstream,
                                      std::decay_t<decltype(item)>>) {
        auto &file = const_cast<std::decay_t<decltype(item)> &>(item);

        std::string content;
        content.assign(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());

        stream_ << content;

        if (delimiter_ != '\0') {
          stream_ << delimiter_;
        }
      } else {
        stream_ << item;
        if (delimiter_ != '\0') {
          stream_ << delimiter_;
        }
      }
    }
  }

private:
  Range range_;
  OutputStream &stream_;
  char delimiter_;
};

template <typename OutputStream> class WriteOperation {
public:
  WriteOperation(OutputStream &stream, char delimiter = '\0')
      : stream_(stream), delimiter_(delimiter) {}

  template <typename Range> auto operator|(Range &&range) const {
    return WriteProxy<Range, OutputStream>(std::forward<Range>(range), stream_,
                                           delimiter_);
  }

private:
  OutputStream &stream_;
  char delimiter_;
};

template <typename OutputStream>
auto Write(OutputStream &stream, char delimiter = '\0') {
  return WriteOperation<OutputStream>(stream, delimiter);
}

template <typename Range, typename OutputStream>
auto operator|(Range &&range, const WriteOperation<OutputStream> &op) {
  return op.operator|(std::forward<Range>(range));
}