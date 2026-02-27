#pragma once
#include <string>
#include <vector>
#include <filesystem>

class FileUtil
{
    public:
        static std::string readFileToString(const std::filesystem::path &path);
        static std::vector<char> readFileToCharVector(const std::filesystem::path &path);

        static bool Write(const std::string_view data, const std::filesystem::path &path, bool overwrite = false);
        static inline bool Write(const std::vector<char> data, const std::filesystem::path &path, bool overwrite = false)
        {
            return Write(data.data(), path, overwrite);
        }

        static bool Delete(const std::filesystem::path &path);

        static std::vector<std::string> getFiles(const std::filesystem::path &directoryPath);
        static std::vector<std::string> getFilesRecursive(const std::filesystem::path &directoryPath);
        static std::vector<std::string> getSubdirectories(const std::filesystem::path &directoryPath);

        static bool fileExists(const std::filesystem::path &path);
        static bool directoryExists(const std::filesystem::path &directoryPath);
        static inline bool folderExists(const std::filesystem::path &directoryPath) // Alias for directoryExists
        {
            return directoryExists(directoryPath);
        }

        static bool Move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath, bool createDir = false, bool replace = false);
};