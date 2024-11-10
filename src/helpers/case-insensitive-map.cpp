#include "case-insensitive-map.hpp"
#include <algorithm>

namespace VpkParser {
  bool CaseInsensitiveComparer::operator()(const std::string& lhs, const std::string& rhs) const noexcept {
    return std::ranges::equal(lhs, rhs, [](const unsigned char a, const unsigned char b) {
      return std::tolower(a) == std::tolower(b);
    });
  }
}
