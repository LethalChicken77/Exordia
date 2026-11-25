#include "file_util.hpp"
#include "console.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

template<FileBufferConcept T>
T FileUtil::ReadFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Console::error("File not found: " + path.string(), "File Util");
        return T{};
    }

    
    size_t fileSize = static_cast<size_t>(file.tellg());

    T content = [fileSize]
    {
        if constexpr(std::same_as<T, std::string>)
            return T(fileSize + 1, '\0');
        else
            return T(fileSize + 1);
    }();
    
    file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
    file.read(reinterpret_cast<char*>(&content[0]), fileSize);   // Read the file content into the string

    if (!file)
    {
        Console::error("Could not read file: " + path.string(), "File Util");
        return T{};
    }

    content[fileSize] = {}; // Null termination so string pointers can be passed to C functions.

    return content;
}

/// @brief Write data to a file as a string of bytes
/// @param data String view of data, accepts string, byte array, etc.
/// @param path Path to write to
/// @param overwrite Replace existing file with new data
/// @return True on success, false on failure
bool FileUtil::Write(const std::string_view data, const fs::path &path, bool overwrite)
{
    if(fs::exists(path))
    {
        if(!overwrite)
        {
            Console::errorf("File already exists at \"{}\". Run with overwrite = true to force.", path.string(), "FileUtil");
            return false;
        }
    }

    ofstream writeStream(path, ios::out | ios::trunc);
    if(!writeStream.is_open())
    {
        Console::errorf("Failed to open file stream at \"{}\"", path.string(), "FileUtil");
        return false;
    }
    writeStream << data;
    if(writeStream.fail())
    {
        Console::errorf("Write to {} failed", path.string(), "FileUtil");
        return false;
    }
    return true;
}

/// @brief Attempt to delete a file or directory
/// @param path File or directory to delete
/// @return True if file deleted (or not found), false on failure
bool FileUtil::Delete(const fs::path &path)
{
    error_code err;
    fs::remove(path, err);
    if(err)
    {
        Console::errorf("Failed to delete \"{}\": {}", path.string(), err.message(), "FileUtil");
        return false;
    }
    return true;
}

/// @brief Get a list of files in the specified directory
/// @param directoryPath Directory to search
/// @return 
std::vector<std::string> FileUtil::GetFiles(const fs::path &directoryPath)
{
    std::vector<std::string> paths;
    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }
    return paths;
}

std::vector<std::string> FileUtil::GetFilesRecursive(const std::filesystem::path &directoryPath)
{
    std::vector<std::string> paths;
    
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }

    std::vector<std::string> subDirs = GetSubdirectories(directoryPath);
    for(const std::string &subDir : subDirs)
    {
        std::vector<std::string> subPaths = GetFilesRecursive(subDir);
        paths.insert(paths.end(), subPaths.begin(), subPaths.end());
    }
    return paths;
}

std::vector<std::string> FileUtil::GetSubdirectories(const std::filesystem::path &directoryPath)
{
    std::vector<std::string> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_directory())
            paths.push_back(entry.path().string());
    }
    return paths;
}

/// @brief Move or rename a file
/// @param oldPath Source file path
/// @param newPath Destination file path
/// @param createDir Create a directory at the new path if it doesn't exist
/// @param replace Replace destination file
/// @return 
bool FileUtil::Move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath, bool createDir, bool replace)
{
    error_code err;
    if(FileExists(newPath) && !replace)
    {
        if(replace)
        {
            Delete(newPath);
        }
        else
        {
            Console::errorf("Cannot move file: File already exists at {}. Run with replace = true to force move.", newPath.string(), "FileUtil");
            return false;
        }
    }

    filesystem::rename(oldPath, newPath, err);
    if(err)
    {
        Console::errorf("Failed to move file \"{}\" to \"{}\": {}", oldPath.string(), newPath.string(), err.message(), "FileUtil");
        return false;
    }
    return true;    
}

template string FileUtil::ReadFile<string>(const std::filesystem::path &path);
template std::vector<char> FileUtil::ReadFile<std::vector<char>>(const std::filesystem::path &path);
template std::vector<uint8_t> FileUtil::ReadFile<std::vector<uint8_t>>(const std::filesystem::path &path);
template std::vector<std::byte> FileUtil::ReadFile<std::vector<std::byte>>(const std::filesystem::path &path);