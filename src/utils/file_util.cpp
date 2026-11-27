#include "file_util.hpp"
#include "console.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <array>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace FileUtil
{

template<FileBufferConcept T>
T ReadFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        // Console::error("File not found: " + path.string(), "File Util");
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
        // Console::error("Could not read file: " + path.string(), "File Util");
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
bool Write(const std::string_view data, const fs::path &path, bool overwrite)
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
bool Delete(const fs::path &path)
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
std::vector<std::string> GetFiles(const fs::path &directoryPath)
{
    std::vector<std::string> paths;
    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath))
    {
        if(entry.is_regular_file())
            paths.push_back(entry.path().string());
    }
    return paths;
}

std::vector<std::string> GetFilesRecursive(const std::filesystem::path &directoryPath)
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

std::vector<std::string> GetSubdirectories(const std::filesystem::path &directoryPath)
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
bool Move(const std::filesystem::path &oldPath, const std::filesystem::path &newPath, bool createDir, bool replace)
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

template string ReadFile<string>(const std::filesystem::path &path);
template std::vector<char> ReadFile<std::vector<char>>(const std::filesystem::path &path);
template std::vector<uint8_t> ReadFile<std::vector<uint8_t>>(const std::filesystem::path &path);
template std::vector<std::byte> ReadFile<std::vector<std::byte>>(const std::filesystem::path &path);

// File identification


struct MagicEntry
{
    std::span<const unsigned char> Magic;
    FileType Type;
};

struct ExtensionEntry
{
    std::string_view Extension;
    FileType Type;
};

constexpr unsigned char SpirvMagic[] = { 0x03, 0x02, 0x23, 0x07 };
constexpr unsigned char SpirvMagicBE[] = { 0x07, 0x23, 0x02, 0x03 };
constexpr unsigned char JpegMagic[] = { 0xFF, 0xD8, 0xFF };
constexpr unsigned char PngMagic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
constexpr unsigned char ExrMagic[] = { 0x76, 0x2F, 0x31, 0x01 };

constexpr std::array MagicEntries
{
    MagicEntry { SpirvMagic, FileType::Spirv },
    MagicEntry { SpirvMagicBE, FileType::Spirv },
    MagicEntry { JpegMagic, FileType::Jpeg },
    MagicEntry { PngMagic, FileType::Png },
    MagicEntry { ExrMagic, FileType::Exr }
};

constexpr std::array ExtensionEntries
{
    ExtensionEntry { ".txt", FileType::Txt },
    ExtensionEntry { ".slang", FileType::Slang },
    ExtensionEntry { ".tui", FileType::Tui },
    ExtensionEntry { ".yaml", FileType::Yaml },
    ExtensionEntry { ".yml", FileType::Yaml },
    ExtensionEntry { ".toml", FileType::Toml },
    ExtensionEntry { ".json", FileType::Json },
    ExtensionEntry { ".spv", FileType::Spirv },
    ExtensionEntry { ".jpeg", FileType::Jpeg },
    ExtensionEntry { ".jpg", FileType::Jpeg },
    ExtensionEntry { ".png", FileType::Png },
    ExtensionEntry { ".exr", FileType::Exr }
};

constexpr char ToLower(char c)
{
    return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

constexpr bool EqualsIgnoreCase(std::string_view a, std::string_view b)
{
    if(a.size() != b.size())
        return false;

    for(size_t i = 0; i < a.size(); i++)
    {
        if(ToLower(a[i]) != ToLower(b[i]))
            return false;
    }

    return true;
}

FileType IdentifyFileType(std::string_view path, std::span<const char> contents)
{
    // Check magic numbers first.
    for(const MagicEntry& entry : MagicEntries)
    {
        if(contents.size() < entry.Magic.size())
            continue;

        if(std::memcmp(contents.data(), entry.Magic.data(), entry.Magic.size()) == 0)
            return entry.Type;
    }

    // Extract the file extension.
    size_t filenameStart = path.find_last_of("/\\");
    filenameStart = filenameStart == std::string_view::npos ? 0 : filenameStart + 1;

    size_t extensionStart = path.find_last_of('.');

    if(extensionStart == std::string_view::npos || extensionStart <= filenameStart)
        return FileType::Unknown;

    std::string_view extension = path.substr(extensionStart);

    // Fall back to extension matching.
    for(const ExtensionEntry& entry : ExtensionEntries)
    {
        if(EqualsIgnoreCase(extension, entry.Extension))
            return entry.Type;
    }

    return FileType::Unknown;
}
}