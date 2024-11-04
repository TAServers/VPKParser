#pragma once

#include "structs/directory-entry.hpp"
#include <filesystem>
#include <functional>
#include <span>

namespace VpkParser {
  class Vpk {
  public:
    Vpk() = default;

    explicit Vpk(const std::span<std::byte>& data);

    const std::vector<std::byte>& getPreloadData(const std::filesystem::path& path) const;

    std::vector<std::byte> readFile(
      const std::filesystem::path& path,
      const std::function<std::vector<std::byte>(uint16_t archive, uint32_t offset, uint32_t size)>& readFromArchive
    ) const;

    bool fileExists(const std::filesystem::path& path) const;

  private:
    struct File {
      uint16_t archiveIndex;

      uint32_t offset;

      uint32_t size;

      std::vector<std::byte> preloadData;
    };

    std::unordered_map<std::filesystem::path, File> files;
  };
}
