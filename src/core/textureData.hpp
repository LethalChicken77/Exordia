#pragma once
#include <vector>
#include <cstdint>

#include "primitives/color.hpp"

namespace core
{
    class TextureData
    {
        public:
            TextureData() = default;
            ~TextureData() = default;

            const uint8_t* GetDataPtr() const { return data.data(); }
            const std::vector<uint8_t>& GetData() const { return data; }
            const size_t GetSize() const { return data.size(); }

            // TODO: Getters for other types
            const uint8_t GetByte(uint32_t x, uint32_t y, uint32_t z = 0) const;
            const uint16_t GetShort(uint32_t x, uint32_t y, uint32_t z = 0) const;
            const uint32_t GetInt(uint32_t x, uint32_t y, uint32_t z = 0) const;
            const float GetFloat16(uint32_t x, uint32_t y, uint32_t z = 0) const;
            const float GetFloat(uint32_t x, uint32_t y, uint32_t z = 0) const;
            const Color GetColor(uint32_t x, uint32_t y, uint32_t z = 0) const;

            const uint32_t GetWidth() const { return width; }
            const uint32_t GetHeight() const { return height; }
            const uint32_t GetDepth() const { return depth; }
            const uint32_t GetTypeSize() const { return typeSize; }
            const uint32_t GetChannelCount() const { return channelCount; }
            const uint32_t GetPixelSize() const { return typeSize * channelCount; }

        private:
            std::vector<uint8_t> data{}; // Data stored in binary format, can be interpreted as whatever
            // std::unique_ptr<Buffer> buffer;

            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;

            uint32_t typeSize = 8; // Size of a single channel in bytes
            uint32_t channelCount = 4; // Number of channels per pixel
    };
} // namespace core