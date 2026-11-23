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


SpriteDrawCall::SpriteDrawCall(const Sprite& sprite, const glm::mat4& transform, MaterialHandle _materialHandle, id_t objID)
    : SpriteDrawCall(sprite, sprite.GetSimpleSize(), {&transform, 1}, _materialHandle, objID) {}
SpriteDrawCall::SpriteDrawCall(const Sprite& sprite, const std::span<const glm::mat4> _transforms, MaterialHandle _materialHandle, id_t objID)
    : SpriteDrawCall(sprite, sprite.GetSimpleSize(), _transforms, _materialHandle, objID) {}

SpriteDrawCall::SpriteDrawCall(const Sprite& sprite, glm::vec2 size, const glm::mat4& transform, MaterialHandle _materialHandle, id_t objID)
    : SpriteDrawCall(sprite, size, {&transform, 1}, _materialHandle, objID) {}
SpriteDrawCall::SpriteDrawCall(const Sprite& sprite, glm::vec2 size, const std::span<const glm::mat4> _transforms, MaterialHandle _materialHandle, id_t objID)
    : SpriteDrawCall(SpriteDrawInfo{sprite, size, SpriteDrawMode::Stretch}, _transforms, _materialHandle, objID) {}

SpriteDrawCall::SpriteDrawCall(const SpriteDrawInfo& _drawInfo, const glm::mat4& transform, MaterialHandle _materialHandle, id_t objID)
    : SpriteDrawCall(_drawInfo, {&transform, 1}, _materialHandle, objID) {}
SpriteDrawCall::SpriteDrawCall(const SpriteDrawInfo& _drawInfo, const std::span<const glm::mat4> _transform, MaterialHandle _materialHandle, id_t objID)
    : drawInfo(_drawInfo), objectID(objID), transforms{_transform.begin(), _transform.end()}, materialHandle{_materialHandle}
{
    createInstanceBuffer(graphicsData->GetBackend().GetDevice()); // TODO: Lazy creation of instance buffer to better support batching
}


void SpriteDrawCall::Bind(vk::CommandBuffer commandBuffer)
{
    vk::Buffer buffers[] = {instanceBuffer->GetBuffer()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(2, 1, buffers, offsets);
        
    commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    commandBuffer.setPrimitiveRestartEnable(false);
}

void SpriteDrawCall::Draw(vk::CommandBuffer commandBuffer) const
{
    int quadCount = drawInfo.drawMode == SpriteDrawMode::Stretch ? drawInfo.sprite.Get9SliceQuadCount() : drawInfo.sprite.GetTiledQuadCount(drawInfo.size);
    commandBuffer.draw(quadCount * 6, transforms.size(), 0, 0);
}


void SpriteDrawCall::createInstanceBuffer(internal::Device& device)
{
    uint32_t instanceCount = transforms.size();
    uint32_t instanceSize = sizeof(SpriteInstanceInfo);
    VkDeviceSize bufferSize = instanceSize * instanceCount;

    std::vector<SpriteInstanceInfo> instanceData{};
    const Sprite& sprite = drawInfo.sprite;
    instanceData.resize(transforms.size());
    // Console::debugf("Sprite Pos: {}", sprite.GetPos());
    // Console::debugf("Sprite Size: {}", sprite.GetSize());
    glm::vec2 uvMin = sprite.GetUVMin();
    glm::vec2 uvMax = sprite.GetUVMax();
    glm::vec2 textureSize = (glm::vec2)sprite.GetTextureSize();
    glm::vec4 border{
        sprite.GetUVLeft(),
        sprite.GetUVRight(),
        sprite.GetUVTop(),
        sprite.GetUVBottom()
    };
    for(int i = 0; i < instanceData.size(); i++)
    {
        SpriteInstanceInfo& instance = instanceData[i];
        instance.Model = transforms[i];
        instance.Size = drawInfo.size;
        instance.Pivot = sprite.GetPivot();
        instance.UVMin = uvMin;
        instance.UVMax = uvMax;
        instance.Border = border;
        instance.TextureSize = (glm::vec2)sprite.GetTextureSize();
        instance.PixelsPerUnit = sprite.GetTexturePPU();
        instance.Tiled = drawInfo.drawMode == SpriteDrawMode::Tile;
    }

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        device,
        instanceSize,
        instanceCount
    );

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)instanceData.data(), bufferSize);

    instanceBuffer = std::make_unique<Buffer>(
        device,
        instanceSize,
        instanceCount,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    instanceBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
}
} // namespace graphics
