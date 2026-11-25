#pragma once

#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "core/resources/mesh.hpp"
#include "engine_types.hpp"
#include "graphics/api/resources/sprite.hpp"

#include "graphics/resources/layouts/vertex_layout.hpp"

namespace graphics
{
    struct SpriteInstanceInfo
    {
        glm::mat4 Model = glm::mat4(1);
        glm::vec2 Size = {1.f, 1.f};
        glm::vec2 Pivot = {0.f, 0.f};
        glm::vec2 UVMin = {0.f, 0.f};
        glm::vec2 UVMax = {1.f, 1.f};
        glm::vec4 Border{};
        glm::vec2 TextureSize;
        float PixelsPerUnit;
        int TextureIndex = 0; // TODO: Implement with bindless textures
        uint32_t Tiled;
    };
    struct SpriteDrawCall
    {
        SpriteDrawCall(const Sprite& sprite, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        SpriteDrawCall(const Sprite& sprite, const std::span<const glm::mat4> _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        SpriteDrawCall(const Sprite& sprite, glm::vec2 size, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        SpriteDrawCall(const Sprite& sprite, glm::vec2 size, const std::span<const glm::mat4> _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        SpriteDrawCall(const SpriteDrawInfo& drawInfo, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        SpriteDrawCall(const SpriteDrawInfo& drawInfo, const std::span<const glm::mat4> _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        SpriteDrawInfo drawInfo;
        uint32_t objectID;
        MaterialHandle materialHandle;
        std::vector<glm::mat4> transforms{};
        std::unique_ptr<Buffer> instanceBuffer{};

        void Bind(vk::CommandBuffer commandBuffer);
        void Draw(vk::CommandBuffer commandBuffer) const;
    private:
        void createInstanceBuffer(internal::Device &device);
        // void UpdateInstanceBuffer(const std::vector<glm::mat4> &transforms);
    };
}