// #pragma once

// #include "buffer.hpp"

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/glm.hpp>
// #include <vector>
// #include <memory>
// #include <string>
// #include <iostream>

// #include "graphics/containers.hpp"
// #include "core/mesh.hpp"

// namespace graphics
// {
//     class GraphicsMesh // TODO: Replace with graphics.draw(Mesh, Material)
//     {
//     public:
//         static std::vector<VkVertexInputBindingDescription> getVertexBindingDescriptions();
//         static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

//         GraphicsMesh(core::MeshData* meshptr);
//         ~GraphicsMesh();

//         GraphicsMesh(const GraphicsMesh&) = delete;
//         GraphicsMesh& operator=(const GraphicsMesh&) = delete;

//         void bind(VkCommandBuffer commandBuffer, const std::unique_ptr<Buffer> &instanceBuffer); // TODO: Remove in favor of graphics.draw(Mesh)
//         void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount);

//         void createBuffers();
//         void createInstanceBuffer(const std::vector<glm::mat4> &transforms);
//         void updateInstanceBuffer(const std::vector<glm::mat4> &transforms);

//     private:
//         std::unique_ptr<Buffer> vertexBuffer{};
//         uint32_t vertexCount;
//         bool useIndexBuffer = true;
//         std::unique_ptr<Buffer> indexBuffer{};
//         uint32_t indexCount;

//         core::MeshData* meshPtr;

//         void createVertexBuffer();
//         void createIndexBuffer();
        
//         void loadModelFromObj(const std::string& filename);
//     };
    
//     struct MeshRenderData
//     {
//         MeshRenderData(id_t id, const glm::mat4& _transform, uint32_t materialID = 0, id_t objID = -1)
//             : meshID(id), objectID(objID), transforms{_transform}, materialIndex{materialID}, instanceBuffer{createInstanceBuffer(transforms)} {}
//         MeshRenderData(id_t id, const std::vector<glm::mat4>& _transform, uint32_t materialID = 0, id_t objID = -1)
//             : meshID(id), objectID(objID), transforms{_transform}, materialIndex{materialID}, instanceBuffer{createInstanceBuffer(transforms)} {}

//         id_t meshID;
//         uint32_t objectID;
//         uint32_t materialIndex;
//         std::vector<glm::mat4> transforms{};
//         std::unique_ptr<Buffer> instanceBuffer{};
//     };
// }