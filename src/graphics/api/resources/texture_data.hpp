#pragma once
#include <vector>
#include <cstdint>

#include "primitives/color.hpp"
#include "graphics/api/texture_properties.hpp"

#include "nd_vector.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{
    class TextureData
    {
        public:
            TextureData() = default;
            ~TextureData() = default;

            const uint8_t* GetDataPtr() const { return data.data(); }
            const std::vector<uint8_t>& GetData() const { return data; }
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

            inline uint32_t GetWidth() const { return width; }
            inline uint32_t GetHeight() const { return height; }
            inline uint32_t GetDepth() const { return depth; }
            inline uint32_t GetTypeSize() const { return typeSize; }
            inline uint32_t GetChannelCount() const { return channelCount; }
            inline uint32_t GetPixelSize() const { return typeSize * channelCount; }

            TextureConfig properties{};

            TextureHandle graphicsHandle{};

        private:
            std::vector<uint8_t> data; // Data stored in binary format, can be interpreted as whatever
            // Vector2D<uint8_t> data;
            // std::unique_ptr<Buffer> buffer;

            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;

            uint32_t typeSize = 4; // Size of a single channel in bytes
            uint32_t channelCount = 4; // Number of channels per pixel
    };
} // namespace core