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
    class GraphicsSprite
    {
    public:

        GraphicsSprite(const core::MeshData* meshptr);
        GraphicsSprite(internal::Device &device, const core::MeshData* meshptr);
        ~GraphicsSprite();

        GraphicsSprite(const GraphicsSprite&) = delete;
        GraphicsSprite& operator=(const GraphicsSprite&) = delete;

        void Bind(vk::CommandBuffer commandBuffer, const VertexLayout& vertexLayout, const std::unique_ptr<Buffer> &instanceBuffer);
        void Draw(vk::CommandBuffer commandBuffer, uint32_t instanceCount) const;

        void RecreateBuffers();

    private:
        internal::Device &m_device;

        const Sprite* m_spriteData;
        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;

        std::unique_ptr<Buffer> m_positionBuffer{};
        // std::unique_ptr<Buffer> m_vertexBuffer{};
        std::vector<Buffer> m_vertexBuffers{};
        std::unordered_map<VertexLayout, uint32_t, VertexLayout::Hash> m_vertexBufferIndex{};
        bool m_useIndexBuffer = true;
        MeshConfig m_config{};
        std::unique_ptr<Buffer> m_indexBuffer{};

        void createPositionBuffer();
        void createVertexBuffer(const VertexLayout& vertexLayout);
        void createIndexBuffer();
        
        void loadModelFromObj(const std::string& filename);
    };
    
    struct SpriteDrawInfo
    {
        SpriteDrawInfo(MeshHandle handle, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        SpriteDrawInfo(MeshHandle handle, const std::vector<glm::mat4>& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        MeshHandle handle;
        uint32_t objectID;
        MaterialHandle materialHandle;
        std::vector<glm::mat4> transforms{};
        std::unique_ptr<Buffer> instanceBuffer{};

    private:
        std::unique_ptr<Buffer> createInstanceBuffer(internal::Device &device, const std::vector<glm::mat4> &transforms);
        // void UpdateInstanceBuffer(const std::vector<glm::mat4> &transforms);
    };
}