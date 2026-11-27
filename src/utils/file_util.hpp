#pragma once
#include <concepts>
#include <span>
#include <string>
#include <vector>
#include <filesystem>


enum class FileType
{
    Unknown = 0,

    // Text formats
    Txt = 1,
    Slang = 2,
    Tui = 3,
    Yaml = 4,
    Toml = 5,
    Json = 6,

    // Binary formats
    Spirv = 50,

    // Image formats
    Jpeg = 100,
    Png = 101,
    Exr = 102

    // Audio formats

    // Video formats
};

namespace FileUtil
{

template<typename T>
concept FileBufferConcept =
    std::same_as<T, std::string> ||
    std::same_as<T, std::vector<char>> ||
    std::same_as<T, std::vector<uint8_t>> ||
    std::same_as<T, std::vector<std::byte>>;
template<FileBufferConcept T>
[[nodiscard]] T ReadFile(const std::filesystem::path &path);

bool Write(std::string_view data, const std::filesystem::path &path, bool overwrite = false);

bool Delete(const std::filesystem::path &path);

[[nodiscard]] std::vector<std::string> GetFiles(const std::filesystem::path &directoryPath);
[[nodiscard]] std::vector<std::string> GetFilesRecursive(const std::filesystem::path &directoryPath);
[[nodiscard]] std::vector<std::string> GetSubdirectories(const std::filesystem::path &directoryPath);

inline bool FileExists(const std::filesystem::path &path) 
{ return std::filesystem::is_regular_file(path); }
inline bool DirectoryExists(const std::filesystem::path &directoryPath)
{ return std::filesystem::is_directory(directoryPath); }
inline bool FolderExists(const std::filesystem::path &directoryPath) // Alias for directoryExists
{ return DirectoryExists(directoryPath); }

bool Move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath, bool createDir = false, bool replace = false);

[[nodiscard]] FileType IdentifyFileType(std::string_view path, std::span<const char> contents);
} // namespace FileUtil