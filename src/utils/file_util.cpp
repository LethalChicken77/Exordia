#include "file_util.hpp"
#include "console.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

std::string FileUtil::readFileToString(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Console::error("File not found: " + path.string(), "File Util");
        return "";
    }

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::string content(fileSize, '\0'); // Create a string of the appropriate size
    
    file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
    file.read(&content[0], fileSize);   // Read the file content into the string

    if (!file)
    {
        Console::error("Could not read file: " + path.string(), "File Util");
        return "";
    }

    return content;
}

std::vector<char> FileUtil::readFileToCharVector(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Console::error("File not found: " + path.string(), "File Util");
        return std::vector<char>();
    }

    
    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> content(fileSize); // Create a string of the appropriate size
    
    file.seekg(0, std::ios::beg);  // Move back to the beginning of the file
    file.read(&content[0], fileSize);   // Read the file content into the string

    if (!file)
    {
        Console::error("Could not read file: " + path.string(), "File Util");
        return std::vector<char>();
    }

    return content;
}

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

bool FileUtil::Delete(const filesystem::path &path)
{
    error_code err;
    filesystem::remove(path, err);
    if(err)
    {
        Console::errorf("Failed to remove file at \"{}\"", path.string(), "FileUtil");
        return false;
    }
    return true;
}

std::vector<std::string> FileUtil::getFiles(const std::filesystem::path &directoryPath)
{
    std::vector<std::string> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }
    return paths;
}

std::vector<std::string> FileUtil::getFilesRecursive(const std::filesystem::path &directoryPath)
{
    std::vector<std::string> paths;
    
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }

    std::vector<std::string> subDirs = getSubdirectories(directoryPath);
    for(const std::string &subDir : subDirs)
    {
        std::vector<std::string> subPaths = getFilesRecursive(subDir);
        paths.insert(paths.end(), subPaths.begin(), subPaths.end());
    }
    return paths;
}

std::vector<std::string> FileUtil::getSubdirectories(const std::filesystem::path &directoryPath)
{
    std::vector<std::string> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if(entry.is_directory())
            paths.push_back(entry.path().string());
    }
    return paths;
}

bool FileUtil::fileExists(const std::filesystem::path &path)
{
    return std::filesystem::is_regular_file(path);
}

bool FileUtil::directoryExists(const std::filesystem::path &directoryPath)
{
    return std::filesystem::is_directory(directoryPath);
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
    if(fileExists(newPath) && !replace)
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