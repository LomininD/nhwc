module;

#include <print>
#include <iostream>
#include <optional>
#include <string>
#include <charconv>

export module util;

namespace util {

export template<typename T>
std::optional<T> read_integer(std::istream& stream = std::cin) {
  std::string data;
  T result;

  stream >> data;

  auto [ptr, ec] = std::from_chars(data.data(), data.data() + data.size(), result);
  if (ec == std::errc::invalid_argument) {
    if (data.empty()) {
      std::println(stderr, "error: the value required.");
    } else {
      std::println(stderr, "error: the value '{}' is not allowed.", data);
    }
    return std::nullopt;
  } else if (ec == std::errc::result_out_of_range) {
    std::println(stderr, "error: the value '{}' is out of a range.", data);
    return std::nullopt;
  }

  return result;
}

}  // namespace util
