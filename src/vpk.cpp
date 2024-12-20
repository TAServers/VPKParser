#include "vpk.hpp"
#include "helpers/case-insensitive-map.hpp"
#include "helpers/offset-data-view.hpp"
#include "structs/directory-entry.hpp"
#include "structs/headers.hpp"
#include <set>

namespace VpkParser {
  using namespace Errors;
  using namespace Structs;

  namespace {
    constexpr uint32_t FILE_SIGNATURE = 0x55aa1234;
    const std::set<uint32_t> SUPPORTED_VERSIONS = {1, 2};
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
    while (true) {
      auto extension = dataView.parseString(offset, "Failed to parse extension");
      offset += extension.length() + 1;
      if (extension.empty()) {
        break;
      }
      extension = "." + extension;

      files.emplace(extension, CaseInsensitiveMap<CaseInsensitiveMap<File>>());

      while (true) {
        const auto directory = dataView.parseString(offset, "Failed to parse directory");
        offset += directory.length() + 1;
        if (directory.empty()) {
          break;
        }

        files.at(extension).emplace(directory, CaseInsensitiveMap<File>());

        while (true) {
          const auto filename = dataView.parseString(offset, "Failed to parse filename");
          offset += filename.length() + 1;
          if (filename.empty()) {
            break;
          }

          const auto directoryInfo =
            dataView.parseStruct<DirectoryEntry>(offset, "Failed to parse directory entry").first;
          offset += sizeof(DirectoryEntry);

          files.at(extension).at(directory).emplace(
            filename,
            File{
              .archiveIndex = directoryInfo.archiveIndex,
              .offset = directoryInfo.entryOffset,
              .size = directoryInfo.entrySize,
              .preloadData = dataView.parseStructArrayWithoutOffsets<std::byte>(
                offset, directoryInfo.preloadDataSize, "Failed to parse preload data"
              ),
            }
          );

          offset += directoryInfo.preloadDataSize;
        }
      }
    }
  }

  const std::vector<std::byte>& Vpk::getPreloadData(const std::filesystem::path& path) const {
    return getFileMetadata(path).preloadData;
  }

  std::vector<std::byte> Vpk::readFile(
    const std::filesystem::path& path,
    const std::function<std::vector<std::byte>(uint16_t archive, uint32_t offset, uint32_t size)>& readFromArchive
  ) const {
    const auto& fileInfo = getFileMetadata(path);
    const auto& archiveData = readFromArchive(fileInfo.archiveIndex, fileInfo.offset, fileInfo.size);

    std::vector<std::byte> fileData;
    fileData.reserve(fileInfo.preloadData.size() + fileInfo.size);

    fileData.insert(fileData.begin(), fileInfo.preloadData.begin(), fileInfo.preloadData.end());
    fileData.insert(fileData.end(), archiveData.begin(), archiveData.end());

    return std::move(fileData);
  }

  bool Vpk::fileExists(const std::filesystem::path& path) const {
    return files.contains(path.extension().generic_string()) && //
      files.at(path.extension().generic_string()).contains(path.parent_path().generic_string()) && //
      files.at(path.extension().generic_string())
        .at(path.parent_path().generic_string())
        .contains(path.stem().generic_string());
  }

  const Vpk::File& Vpk::getFileMetadata(const std::filesystem::path& path) const {
    return files.at(path.extension().generic_string()).at(path.parent_path().generic_string()).at(path.stem().generic_string());
  }
}
