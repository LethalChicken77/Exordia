#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace graphics
{

class VertexLayout
{
public:
    enum class AttributeType : uint8_t
    {
        Position,
        Normal,
        Color,
        Tangent,
        Bitangent,
        UV,
        Other
    };

    enum class AttributeLayout : uint8_t
    {
        Standard, // Float components
        Half, // Half components
        UInt8, // 8 bit components, good for colors
        Octahedral // Octahedral mapping, good for directions
    };
    
    struct Attribute
    {
        AttributeType type{};
        AttributeLayout layout{};
        uint8_t componentCount = 4;
        uint8_t index = 0;

        uint32_t GetFormat() const; // Returns a VkFormat as a uint32_t. This is to avoid including the vulkan header.

        bool operator==(const Attribute& other) const
        {
            return type == other.type && layout == other.layout && componentCount == other.componentCount;
        }

        struct Hash
        {
            uint32_t operator()(const Attribute& a) const
            {
                uint32_t result = 0;
                result |= (uint32_t)a.type << 24 
                    | (uint32_t)a.layout << 16
                    | (uint32_t)a.componentCount << 8
                    | (uint32_t)a.index;
                return result;
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

    const std::vector<Attribute> &GetAttributes() const { return vertexAttributes; }
    
    const std::vector<VkVertexInputBindingDescription> GetVertexBindingDescriptions() const;
    const std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions() const;

private:
    std::vector<Attribute> vertexAttributes;
    std::unordered_map<std::string, uint32_t> vertexIndex{};
};

} // namespace graphics