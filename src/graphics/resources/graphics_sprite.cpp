#include "graphics_sprite.hpp"
#include "graphics/graphics_data.hpp"
#include "utils/packing.hpp"
#include "graphics/utils/packing.hpp"
#include "utils/debug.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>

using Vertex = core::MeshData::Vertex;
using VertexNoPos = core::MeshData::VertexNoPos;
using VertexPos = core::MeshData::VertexPos;
using Triangle = core::MeshData::Triangle;

namespace graphics
{
// GraphicsSprite::GraphicsSprite(Device& _device, const Builder& builder) : device(_device)
// {
//     createVertexBuffer(builder.vertices);
//     createIndexBuffer(builder.triangles);
// }


SpriteDrawInfo::SpriteDrawInfo(const Sprite& _sprite, const glm::mat4& _transform, MaterialHandle _materialHandle, id_t objID)
    : sprite(_sprite), objectID(objID), transforms{_transform}, materialHandle{_materialHandle}, instanceBuffer{createInstanceBuffer(graphicsData->GetBackend().GetDevice(), _sprite)} {}
SpriteDrawInfo::SpriteDrawInfo(const Sprite& _sprite, const std::span<const glm::mat4> _transform, MaterialHandle _materialHandle, id_t objID)
    : sprite(_sprite), objectID(objID), transforms{_transform.begin(), _transform.end()}, materialHandle{_materialHandle}, instanceBuffer{createInstanceBuffer(graphicsData->GetBackend().GetDevice(), _sprite)} {}


void SpriteDrawInfo::Bind(vk::CommandBuffer commandBuffer)
{
    vk::Buffer buffers[] = {instanceBuffer->GetBuffer()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(2, 1, buffers, offsets);
        
    commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    commandBuffer.setPrimitiveRestartEnable(false);
}

void SpriteDrawInfo::Draw(vk::CommandBuffer commandBuffer) const
{
    // TODO: Compute vertex count from sprite info and 
    commandBuffer.draw(6, transforms.size(), 0, 0);
}


std::unique_ptr<Buffer> SpriteDrawInfo::createInstanceBuffer(internal::Device& device, const Sprite& sprite)
{
    uint32_t instanceCount = transforms.size();
    uint32_t instanceSize = sizeof(SpriteInstanceInfo);
    VkDeviceSize bufferSize = instanceSize * instanceCount;

    std::vector<SpriteInstanceInfo> instanceData{};
    instanceData.resize(transforms.size());
    // Console::debugf("Sprite Pos: {}", sprite.GetPos());
    // Console::debugf("Sprite Size: {}", sprite.GetSize());
    glm::vec2 uvMin = (glm::vec2)sprite.GetPos() / (glm::vec2)sprite.GetTextureSize();
    glm::vec2 uvMax = (glm::vec2)(sprite.GetPos() + sprite.GetSize()) / (glm::vec2)sprite.GetTextureSize();
    glm::vec2 textureSize = (glm::vec2)sprite.GetTextureSize();
    glm::vec2 size = (glm::vec2)sprite.GetSize() / sprite.GetTexturePPU();
    for(int i = 0; i < instanceData.size(); i++)
    {
        SpriteInstanceInfo& instance = instanceData[i];
        instance.Model = transforms[i];
        instance.Size = size;
        instance.Pivot = sprite.GetPivot();
        instance.UVMin = uvMin;
        instance.UVMax = uvMax;

    }

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        device,
        instanceSize,
        instanceCount
    );

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)instanceData.data(), bufferSize);

    std::unique_ptr<Buffer> instanceBuffer = std::make_unique<Buffer>(
        device,
        instanceSize,
        instanceCount,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    instanceBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
    return std::move(instanceBuffer);
}
} // namespace graphics
