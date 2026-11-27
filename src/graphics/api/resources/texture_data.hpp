#pragma once
#include <mdspan>
#include <vector>
#include <cstdint>
#include <memory>
#include <span>

#include "primitives/color.hpp"
#include "graphics/api/texture_properties.hpp"

#include "nd_vector.hpp"
#include "graphics/api/handles.hpp"
#include "ref.hpp"

namespace graphics
{
class TextureData
{
    public:
        std::string name = "";

        TextureData() = default;
        TextureData(uint32_t width, uint32_t height, std::string_view name = "New Texture", TextureConfig props = {});
        TextureData(uint32_t width, uint32_t height, uint32_t depth, std::string_view name = "New Texture", TextureConfig props = {});
        ~TextureData() = default;

        
        bool LoadFromFileSTB(const std::string& path); // Use stb_image to load image
        bool LoadFromFileEXR(std::string_view path, TextureDataType dataType = TextureDataType::Invalid, uint32_t channelSize = 4); // Use tinyexr to load image
        bool LoadSTB(std::span<const uint8_t> data);
        bool LoadEXR(std::span<const uint8_t> data, TextureDataType dataType = TextureDataType::Invalid, uint32_t channelSize = 4);

        std::span<uint8_t> GetData() { return {data.data(), data.size()}; }
        std::span<const uint8_t> GetDataConst() const { return {data.data(), data.size()}; }
        inline size_t GetSize() const { return data.size(); }

        // TODO: Getters for other types
        uint8_t GetByte(uint32_t x, uint32_t y, uint32_t z = 0) const;
        uint16_t GetShort(uint32_t x, uint32_t y, uint32_t z = 0) const;
        uint32_t GetInt(uint32_t x, uint32_t y, uint32_t z = 0) const;
        float GetFloat16(uint32_t x, uint32_t y, uint32_t z = 0) const;
        float GetFloat(uint32_t x, uint32_t y, uint32_t z = 0) const;
        Color GetColor(uint32_t x, uint32_t y, uint32_t z = 0) const;

        template<class T>
        [[nodiscard]] T Get(uint32_t x, uint32_t y, uint32_t z = 0) const
        {
            auto test = std::mdspan(static_cast<T*>(data.data()), x, y, z);
            #ifndef DISABLE_VALIDATION
            if(x >= width || y >= height || z >= depth)
            {
                // Console::error(std::format("Cannot access pixel outside texture bounds: ({}, {}, {}), Size: ({}, {}, {})", x, y, z, width, height, depth), "TextureData");
                return T();
            }
            if(sizeof(T) != GetPixelSize())
            {
                // Console::error(std::format("Type size does not match pixel size: Type size: {}, Pixel size: {}", sizeof(T), GetPixelSize()));
                return T();
            }
            #endif

            // size_t offset = GetPixelSize() * sizeof(T)
        }

        glm::ivec2 GetDimensions() const { return {width, height}; }
        glm::ivec3 GetDimensions3() const { return {width, height, depth}; }
        inline uint32_t GetWidth() const { return width; }
        inline uint32_t GetHeight() const { return height; }
        inline uint32_t GetDepth() const { return depth; }
        inline uint32_t GetComponentSize() const { return properties.format.channelSize; }
        inline uint32_t GetChannelCount() const { return properties.format.channelCount; }
        inline uint32_t GetPixelSize() const { return properties.format.PixelSize(); }

        TextureConfig properties{};

        TextureHandle graphicsHandle{};

        [[nodiscard]] inline bool IsValid() const
        {
            return data.size() > 0;
        }

    private:
        std::vector<uint8_t> data{}; // Data stored in binary format, can be interpreted as whatever
        // Vector2D<uint8_t> data;
        // std::unique_ptr<Buffer> buffer;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
};
using Texture = Ref<TextureData>;
} // namespace core