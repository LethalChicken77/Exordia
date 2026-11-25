#pragma once
#include <concepts>
#include <string>
#include <vector>
#include <filesystem>

template<typename T>
concept FileBufferConcept =
    std::same_as<T, std::string> ||
    std::same_as<T, std::vector<char>> ||
    std::same_as<T, std::vector<uint8_t>> ||
    std::same_as<T, std::vector<std::byte>>;
class FileUtil
{
    public:
        template<FileBufferConcept T>
        static T ReadFile(const std::filesystem::path &path);

        static bool Write(const std::string_view data, const std::filesystem::path &path, bool overwrite = false);
        static inline bool Write(const std::vector<char> data, const std::filesystem::path &path, bool overwrite = false)
        {
            return Write(data.data(), path, overwrite);
        }

        static bool Delete(const std::filesystem::path &path);

        static std::vector<std::string> GetFiles(const std::filesystem::path &directoryPath);
        static std::vector<std::string> GetFilesRecursive(const std::filesystem::path &directoryPath);
        static std::vector<std::string> GetSubdirectories(const std::filesystem::path &directoryPath);

        static inline bool FileExists(const std::filesystem::path &path) 
        { return std::filesystem::is_regular_file(path); }
        static inline bool DirectoryExists(const std::filesystem::path &directoryPath)
        { return std::filesystem::is_directory(directoryPath); }
        static inline bool FolderExists(const std::filesystem::path &directoryPath) // Alias for directoryExists
        { return DirectoryExists(directoryPath); }

        static bool Move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath, bool createDir = false, bool replace = false);
};