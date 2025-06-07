#pragma once

#include "helpers/case-insensitive-map.hpp"
#include <filesystem>
#include <functional>
#include <optional>
#include <span>

namespace VpkParser {
  struct DirectoryContents {
    /**
     * Subdirectories of the listed directory.
     */
    std::vector<std::filesystem::path> directories;

    /**
     * Files in the listed directory.
     */
    std::vector<std::filesystem::path> files;
  };

  class Vpk {
  public:
    Vpk() = default;

    explicit Vpk(const std::span<std::byte>& data);

    [[nodiscard]] const std::vector<std::byte>& getPreloadData(const std::filesystem::path& path) const;

    std::vector<std::byte> readFile(
      const std::filesystem::path& path,
      const std::function<std::vector<std::byte>(uint16_t archive, uint32_t offset, uint32_t size)>& readFromArchive
    ) const;

    /**
     * Lists the subdirectories and files of the given directory.
     * @param path Path to list.
     * @return Subdirectories and files, or std::nullopt if path does not exist.
     */
    [[nodiscard]] std::optional<DirectoryContents> list(const std::filesystem::path& path) const;

    [[nodiscard]] bool fileExists(const std::filesystem::path& path) const;

  private:
    struct File {
      uint16_t archiveIndex;

      uint32_t offset;

      uint32_t size;

      std::vector<std::byte> preloadData;
    };

    struct PathComponents {
      std::string extension;
      std::string directory;
      std::string filename;
    };

    /**
     * By extension, then directory, then filename.
     */
    CaseInsensitiveMap<CaseInsensitiveMap<CaseInsensitiveMap<File>>> files;

    [[nodiscard]] const File& getFileMetadata(const std::filesystem::path& path) const;

    [[nodiscard]] static PathComponents splitPath(const std::filesystem::path& path);
  };
}
