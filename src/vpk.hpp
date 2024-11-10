#pragma once

#include "helpers/case-insensitive-map.hpp"
#include <filesystem>
#include <functional>
#include <span>

namespace VpkParser {
  class Vpk {
  public:
    Vpk() = default;

    explicit Vpk(const std::span<std::byte>& data);

    [[nodiscard]] const std::vector<std::byte>& getPreloadData(const std::filesystem::path& path) const;

    std::vector<std::byte> readFile(
      const std::filesystem::path& path,
      const std::function<std::vector<std::byte>(uint16_t archive, uint32_t offset, uint32_t size)>& readFromArchive
    ) const;

    [[nodiscard]] bool fileExists(const std::filesystem::path& path) const;

  private:
    struct File {
      uint16_t archiveIndex;

      uint32_t offset;

      uint32_t size;

      std::vector<std::byte> preloadData;
    };

    /**
     * By extension, then directory, then filename.
     */
    CaseInsensitiveMap<CaseInsensitiveMap<CaseInsensitiveMap<File>>> files;

    [[nodiscard]] const File& getFileMetadata(const std::filesystem::path& path) const;
  };
}
