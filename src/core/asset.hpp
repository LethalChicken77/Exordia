#pragma once
#include <string>
#include <filesystem>
#include <span>
#include "ref.hpp"

namespace core
{

enum class AssetStatus : uint8_t
{
    Invalid, // Asset is invalid
    Unloaded, // Asset is not loaded
    Loading, // Asset is in the process of loading
    Ready // Asset is ready for use
};

struct AssetLoadResult
{
    enum Result : uint8_t
    {
        Success,
        // File errors
        FileNotFound,
        CannotOpenFile,
        // Shader errors
        CompilationFailed,
    };
};

class AssetManager;
class Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Asset";

    Asset() {}
    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;
    Asset(Asset&&) = delete;
    Asset& operator=(Asset&&) = delete;
    virtual ~Asset();

    virtual void Load();
    virtual void Unload();

    [[nodiscard]] inline bool IsValid() const { return m_status != AssetStatus::Invalid; }
    [[nodiscard]] inline bool IsReady() const { return m_status == AssetStatus::Ready; }
    [[nodiscard]] inline AssetStatus Status() const { return m_status; }

    [[nodiscard]] inline std::string Path() const { return m_path.string(); }
    [[nodiscard]] inline std::span<const char> Data() const 
    {
        return m_fileLoaded ? std::span<const char>{m_fileData.data(), m_fileData.size()} : std::span<const char>{}; 
    }
    [[nodiscard]] inline std::string_view DataString() const 
    { 
        return m_fileLoaded ? std::string_view{m_fileData.data(), m_fileData.size()} : std::string_view{}; 
    }

protected:
    AssetStatus m_status{};

    /// @brief Load a file into m_fileData
    /// @return False if file load failed
    bool loadFile();
    /// @brief Clear the m_fileData vector
    void unloadFile();
private:
    std::filesystem::path m_path;
    std::vector<char> m_fileData{}; // TODO: Replace with scratch arena assigned by asset manager
    bool m_fileLoaded = false;

    void SetUUID(); // TODO: Implement
    inline void SetPath(std::string_view path) { m_path = path; }

    friend class AssetManager;
};

} // namespace core