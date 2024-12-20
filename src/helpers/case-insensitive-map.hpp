#pragma once

#include <map>
#include <string>

namespace VpkParser {
  struct CaseInsensitiveComparer {
    bool operator()(const std::string& lhs, const std::string& rhs) const noexcept;
  };

  template <typename T> using CaseInsensitiveMap = std::map<std::string, T, CaseInsensitiveComparer>;
}
