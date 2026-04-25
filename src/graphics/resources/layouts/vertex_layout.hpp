#pragma once
#include "var_layout.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "graphics/utils/alignment.hpp"

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace graphics
{
class SlangReflect;
/// @brief Layout of vertex and instance attributes.
class VertexLayout
{
public:
    enum class AttrSemantic : uint8_t
    {
        Other,
        // Vertex attributes
        Position,
        Normal,
        Color,
        Tangent,
        Bitangent,
        UV,

        // Instance attributes
        I_Model
    };

    struct Attribute
    {
        uint8_t location = 255; // Vulkan pipeline binding location.
        uint32_t offset = ~0u; // Offset in bytes into vertex buffer.
        AttrSemantic semantic = AttrSemantic::Other; // Meaning of the field. Used to separate vertex buffers.
        TypeDescription format{};

        inline uint32_t GetVkFormat() const noexcept { return format.GetVkFormat(); }

        std::string ToString() const;
 
        bool operator==(const Attribute& other) const
        {
            return 
                location == other.location &&
                offset == other.offset &&
                semantic == other.semantic;
        }

        struct Hash
        {
            uint32_t operator()(const Attribute& a) const
            {
                uint32_t hash = 0;

                auto Combine = [](uint32_t& h, uint32_t v)
                {
                    h ^= v + 0x9e3779b9u + (h << 6) + (h >> 2);
                };

                Combine(hash, (uint32_t)a.location);
                Combine(hash, a.offset);
                Combine(hash, (uint32_t)a.semantic);
                TypeDescription::Hash formatHash;
                Combine(hash, formatHash(a.format));

                return hash;
            }
        };
    };
public:
    VertexLayout() = default;

    bool operator==(const VertexLayout& other) const
    {
        return vertexAttributes == other.vertexAttributes;
    }

    struct Hash
    {
        uint32_t operator()(const VertexLayout& a) const
        {
            uint32_t result = 2166136261u; // FNV-1a offset basis
            Attribute::Hash hasher{}; // Hash slinging slasher
            for (const Attribute& attr : a.GetAttributes())
            {
                uint32_t h = hasher(attr);
                // FNV-1a combine
                result ^= h;
                result *= 16777619u; 
            }

            return result;
        }
    };

    inline uint32_t GetPositionStride() const 
    {
        for(const Attribute& attr : vertexAttributes)
        {
            if(attr.semantic == AttrSemantic::Position)
            {
                return attr.format.GetSize();
            }
        }
        // return Alignment::AlignUp(bufferEnd, 16); // Seems to be outdated
        return 0;
    }

    inline uint32_t GetStride() const 
    {
        uint32_t bufferEnd = 0;
        for(const Attribute& attr : vertexAttributes)
        {
            uint32_t end = attr.offset + attr.format.GetSize();
            if(end > bufferEnd)
            {
                bufferEnd = end;
            }
        }
        // return Alignment::AlignUp(bufferEnd, 16); // Seems to be outdated
        return Alignment::AlignUp(bufferEnd, 4);
    }

    inline const std::vector<Attribute> &GetAttributes() const { return vertexAttributes; }
    inline uint8_t GetInstanceBaseLocation() const { return instanceBaseLocation; }
    
    const std::vector<VkVertexInputBindingDescription> GetVertexBindingDescriptions() const;
    const std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions() const;

    std::string ToString() const;

private:
    std::vector<Attribute> vertexAttributes{};
    uint8_t instanceBaseLocation = 255;

    friend class SlangReflect;
};

} // namespace graphics