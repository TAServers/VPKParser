#include "vpk.hpp"
#include "helpers/offset-data-view.hpp"
#include "structs/headers.hpp"
#include <set>

namespace VpkParser {
  using namespace Errors;
  using namespace Structs;

  namespace {
    constexpr uint32_t FILE_SIGNATURE = 0x55aa1234;
    constexpr std::set<uint32_t> SUPPORTED_VERSIONS = {1, 2};
  }

  Vpk::Vpk(const std::span<std::byte>& data) {
    const OffsetDataView dataView(data);
    const auto header = dataView.parseStruct<HeaderV1>(0, "Failed to parse base VPK header").first;

    if (header.signature != FILE_SIGNATURE) {
      throw InvalidHeader("VPK signature does not equal 0x55aa1234");
    }

    if (!SUPPORTED_VERSIONS.contains(header.version)) {
      throw UnsupportedVersion("VPK version not supported (supported versions are 1 and 2)");
    }

    size_t offset = header.version == 1 ? sizeof(HeaderV1) : sizeof(HeaderV2);
    std::string extension;
    std::string directory;
    std::string filename;
    while (!(extension = dataView.parseString(offset, "Failed to parse extension")).empty()) {
      offset += extension.length() + 1;

      while (!(directory = dataView.parseString(offset, "Failed to parse directory")).empty()) {
        offset += directory.length() + 1;

        while (!(filename = dataView.parseString(offset, "Failed to parse filename")).empty()) {
          offset += filename.length() + 1;

          const auto directoryInfo =
            dataView.parseStruct<DirectoryEntry>(offset, "Failed to parse directory entry").first;
          offset += sizeof(DirectoryEntry);

          auto path = std::filesystem::path(directory) / filename;
          path.replace_extension(extension);

          files.emplace(
            path,
            File{
              .archiveIndex = directoryInfo.archiveIndex,
              .offset = directoryInfo.entryOffset,
              .size = directoryInfo.entrySize,
              .preloadData = dataView.parseStructArrayWithoutOffsets<std::byte>(
                offset, directoryInfo.preloadDataSize, "Failed to parse preload data"
              ),
            }
          );
        }
      }
    }
  }

  const std::vector<std::byte>& Vpk::getPreloadData(const std::filesystem::path& path) const {
    return files.at(path).preloadData;
  }

  std::vector<std::byte> Vpk::readFile(
    const std::filesystem::path& path,
    const std::function<std::span<std::byte>(uint16_t archive, uint32_t offset, uint32_t size)>& readFromArchive
  ) const {
    const auto& fileInfo = files.at(path);
    const auto& archiveData = readFromArchive(fileInfo.archiveIndex, fileInfo.offset, fileInfo.size);

    std::vector<std::byte> fileData;
    fileData.reserve(fileInfo.preloadData.size() + fileInfo.size);

    fileData.insert(fileData.begin(), fileInfo.preloadData.begin(), fileInfo.preloadData.end());
    fileData.insert(fileData.end(), archiveData.begin(), archiveData.end());

    return std::move(fileData);
  }

  bool Vpk::fileExists(const std::filesystem::path& path) const {
    return files.contains(path);
  }
}
