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

#include "graphics/api/vertex_layout.hpp"

namespace graphics
{
    class GraphicsMesh
    {
    public:

        GraphicsMesh(const core::MeshData* meshptr);
        GraphicsMesh(internal::Device &device, const core::MeshData* meshptr);
        ~GraphicsMesh();

        GraphicsMesh(const GraphicsMesh&) = delete;
        GraphicsMesh& operator=(const GraphicsMesh&) = delete;

        void bind(vk::CommandBuffer commandBuffer, const std::unique_ptr<Buffer> &instanceBuffer) const; // TODO: Remove in favor of graphics.draw(Mesh)
        void draw(vk::CommandBuffer commandBuffer, uint32_t instanceCount) const;

        void createBuffers(const core::MeshData* meshPtr);

    private:
        internal::Device &device;

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        std::unique_ptr<Buffer> positionBuffer{};
        std::unique_ptr<Buffer> vertexBuffer{};
        std::vector<Buffer> vertexBuffers{};
        std::unordered_map<VertexLayout, size_t, VertexLayout::Hash> vertexBufferIndex{};
        bool useIndexBuffer = true;
        MeshConfig config{};
        std::unique_ptr<Buffer> indexBuffer{};

        void createVertexBuffers(const core::MeshData* meshPtr);
        void createIndexBuffer(const core::MeshData* meshPtr);
        
        void loadModelFromObj(const std::string& filename);
    };
    
    struct MeshRenderData
    {
        MeshRenderData(MeshHandle handle, const glm::mat4& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);
        MeshRenderData(MeshHandle handle, const std::vector<glm::mat4>& _transform, MaterialHandle materialHandle = {}, id_t objID = -1);

        MeshHandle handle;
        uint32_t objectID;
        MaterialHandle materialHandle;
        std::vector<glm::mat4> transforms{};
        std::unique_ptr<Buffer> instanceBuffer{};

        std::unique_ptr<Buffer> CreateInstanceBuffer(internal::Device &device, const std::vector<glm::mat4> &transforms);
        // void UpdateInstanceBuffer(const std::vector<glm::mat4> &transforms);
    };
}