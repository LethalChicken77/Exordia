#pragma once

#include "buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "core/mesh.hpp"
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
        float Left = 0.f;
        float Right = 0.f;
        float Top = 0.f;
        float Bottom = 0.f;
        int TextureIndex = 0; // TODO: Implement with bindless textures
        uint32_t Tiled;
    };
    struct SpriteDrawInfo
    {
        SpriteDrawInfo(const Sprite& sprite, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        SpriteDrawInfo(const Sprite& sprite, const std::span<const glm::mat4> _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        Sprite sprite;
        uint32_t objectID;
        MaterialHandle materialHandle;
        std::vector<glm::mat4> transforms{};
        std::unique_ptr<Buffer> instanceBuffer{};

        void Bind(vk::CommandBuffer commandBuffer);
        void Draw(vk::CommandBuffer commandBuffer) const;
    private:
        std::unique_ptr<Buffer> createInstanceBuffer(internal::Device &device, const Sprite& sprite);
        // void UpdateInstanceBuffer(const std::vector<glm::mat4> &transforms);
    };
}