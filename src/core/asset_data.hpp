#pragma once
#include <string>
#include <span>
#include <memory>

#include "object.hpp"
#include "utils/smart_reference.hpp"
// #include "asset_manager.hpp"

namespace core
{
    class AssetManager;
    class AssetData : public Object
    {
    public:
        static constexpr const char* className = "Asset";
        virtual const char* GetClassName() const { return className; }

        AssetData(const AssetData&) = delete;
        AssetData& operator=(const AssetData&) = delete;
        AssetData(AssetData&&) = delete;
        AssetData& operator=(AssetData&&) = delete;

        id_t getUUID() const { return UUID; }
        const std::string &getPath() const { return path; }
        const std::span<const char> getData() const { return std::span<const char>(data.data(), data.size()); }

        void LoadData();

    protected:
        id_t UUID; // Unique file ID assigned by asset manager. Persistent across sessions
        std::string path; // Located based on file ID
        std::string extension; // File extension, used to identify files with no header
        std::vector<char> data;
        using Object::Object;
        
        friend class AssetManager;
        // AssetData(id_t newID) : Object(newID) {};
    };
} // namespace core