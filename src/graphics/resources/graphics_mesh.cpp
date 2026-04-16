#include "graphics_mesh.hpp"
#include "graphics/graphics_data.hpp"

#include <cassert>
#include <cstring>
#include <cstdint>

using Vertex = core::MeshData::Vertex;
using VertexNoPos = core::MeshData::VertexNoPos;
using VertexPos = core::MeshData::VertexPos;
using Triangle = core::MeshData::Triangle;

namespace graphics
{
// GraphicsMesh::GraphicsMesh(Device& _device, const Builder& builder) : device(_device)
// {
//     createVertexBuffer(builder.vertices);
//     createIndexBuffer(builder.triangles);
// }

GraphicsMesh::GraphicsMesh(const core::MeshData* mesh) : device(graphicsData->GetBackend().GetDevice())
{
    vertexCount = mesh->vertices.size();
    indexCount = mesh->triangles.size() * 3;
    createBuffers(mesh);
}

GraphicsMesh::GraphicsMesh(internal::Device &_device, const core::MeshData* mesh) : device(_device)
{
    vertexCount = mesh->vertices.size();
    indexCount = mesh->triangles.size() * 3;
    createBuffers(mesh);
}

GraphicsMesh::~GraphicsMesh(){}

constexpr vk::PrimitiveTopology getVkTopology(PrimitiveTopology t)
{
    switch(t)
    {
    default:
    case PrimitiveTopology::TriangleList:
        return vk::PrimitiveTopology::eTriangleList;
    case PrimitiveTopology::TriangleFan:
        return vk::PrimitiveTopology::eTriangleFan;
    case PrimitiveTopology::TriangleStrip:
        return vk::PrimitiveTopology::eTriangleStrip;
    case PrimitiveTopology::LineList:
        return vk::PrimitiveTopology::eLineList;
    case PrimitiveTopology::LineStrip:
        return vk::PrimitiveTopology::eLineStrip;
    case PrimitiveTopology::PointList:
        return vk::PrimitiveTopology::ePointList;
    
    }
}

void GraphicsMesh::bind(vk::CommandBuffer commandBuffer, const std::unique_ptr<Buffer> &instanceBuffer) const
{
    vk::Buffer buffers[] = {positionBuffer->GetBuffer(), vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer()};
    vk::DeviceSize offsets[] = {0, 0, 0};
    commandBuffer.bindVertexBuffers(0, 3, buffers, offsets);
    if(useIndexBuffer)
        commandBuffer.bindIndexBuffer(indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
        
    commandBuffer.setPrimitiveTopology(getVkTopology(config.primitiveTopology));
    commandBuffer.setPrimitiveRestartEnable(config.enablePrimitiveRestart);
}

void GraphicsMesh::draw(vk::CommandBuffer commandBuffer, uint32_t instanceCount) const
{
    if(useIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, vertexCount, instanceCount, 0, 0);
}

void GraphicsMesh::createVertexBuffers(const core::MeshData* meshPtr)
{
    if(meshPtr == nullptr)
    {
        Console::error("Cannot create vertex buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    const std::vector<Vertex> &vertices = meshPtr->vertices; // Reference to triangles array for easy access

    assert(vertices.size() >= 3 && "Vertex count must be at least 3");
    std::vector<VertexPos> positions{vertices.size()};
    std::vector<VertexNoPos> vertices2{vertices.size()};

    for(uint32_t i = 0; i < vertices.size(); i++)
    {
        const Vertex& vert = vertices[i];
        positions[i].position = vert.position;
        vertices2[i].normal = vert.normal;
        vertices2[i].tangent = vert.tangent;
        vertices2[i].color = vert.color;
        vertices2[i].texCoord = vert.texCoord;
    }

    uint32_t positionSize = sizeof(positions[0]);
    uint32_t vertexSize = sizeof(vertices2[0]); 

    {
        VkDeviceSize bufferSize = positionSize * vertices.size();
        Buffer stagingBuffer{
            device,
            positionSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
    
        stagingBuffer.Map();
        stagingBuffer.WriteData((void *)positions.data(), vertices.size() * positionSize);
    
        positionBuffer = std::make_unique<Buffer>(
            device,
            positionSize,
            vertices.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        positionBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
    }

    {
        VkDeviceSize bufferSize = vertexSize * vertices.size();
        Buffer stagingBuffer{
            device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
    
        stagingBuffer.Map();
        stagingBuffer.WriteData((void *)vertices2.data(), vertices.size() * vertexSize);
    
        vertexBuffer = std::make_unique<Buffer>(
            device,
            vertexSize,
            vertices.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        vertexBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
    }
}

void GraphicsMesh::createIndexBuffer(const core::MeshData* meshPtr)
{
    if(meshPtr == nullptr)
    {
        Console::error("Cannot create index buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    const std::vector<Triangle> &triangles = meshPtr->triangles; // Reference to triangles array for easy access

    indexCount = static_cast<uint32_t>(triangles.size()) * 3;
    useIndexBuffer = indexCount > 0;

    if(!useIndexBuffer)
        return;

    VkDeviceSize bufferSize = sizeof(triangles[0].v0) * indexCount;
    
    uint32_t indexSize = sizeof(triangles[0].v0);

    Buffer stagingBuffer{
        device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)triangles.data());

    indexBuffer = std::make_unique<Buffer>(
        device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    indexBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
}

void GraphicsMesh::createBuffers(const core::MeshData* meshPtr)
{
    createVertexBuffers(meshPtr);
    createIndexBuffer(meshPtr);
}

std::vector<Vertex> generateSierpinski(float edgeLength, int depth)
{
    float root2 = glm::sqrt(2.0f);
    float root3 = glm::sqrt(3.0f);
    glm::vec3 v0 = glm::vec3(0,         0,      root3 / 3.0f) * edgeLength;
    glm::vec3 v1 = glm::vec3(0.5f,      0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v2 = glm::vec3(-0.5f,     0,      -root3 / 6.0f) * edgeLength;
    glm::vec3 v3 = glm::vec3(0.0f, root3 * 0.5f, 0.0f) * edgeLength;
    if(depth <= 0)
    {
        std::vector<Vertex> vertices(12);
        vertices[0].position = v0;
        vertices[1].position = v2;
        vertices[2].position = v1;
        vertices[0].normal = vertices[1].normal = vertices[2].normal 
            = glm::normalize(-glm::cross(v1 - v0, v2 - v0));

        vertices[3].position = v0;
        vertices[4].position = v1;
        vertices[5].position = v3;
        vertices[3].normal = vertices[4].normal = vertices[5].normal 
            = glm::normalize(glm::cross(v1 - v0, v3 - v0));

        vertices[6].position = v0;
        vertices[7].position = v3;
        vertices[8].position = v2;
        vertices[6].normal = vertices[7].normal = vertices[8].normal 
            = glm::normalize(glm::cross(v3 - v0, v2 - v0));

        vertices[9].position = v3;
        vertices[10].position = v1;
        vertices[11].position = v2;
        vertices[9].normal = vertices[10].normal = vertices[11].normal 
            = glm::normalize(glm::cross(v1 - v3, v2 - v3));
        
        for(Vertex& vertex : vertices)
        {
            vertex.position = vertex.position * 1.5f;
        }

        return vertices;
    }
    else
    {
        std::vector<Vertex> vertices0 = generateSierpinski(edgeLength, depth - 1);
        std::vector<Vertex> vertices1 = vertices0;
        std::vector<Vertex> vertices2 = vertices0;
        std::vector<Vertex> vertices3 = vertices0;

        for(Vertex& vertex : vertices0)
        {
            vertex.position = (vertex.position + v0) * 0.5f;
        }
        for(Vertex& vertex : vertices1)
        {
            vertex.position = (vertex.position + v1) * 0.5f;
        }
        for(Vertex& vertex : vertices2)
        {
            vertex.position = (vertex.position + v2) * 0.5f;
        }
        for(Vertex& vertex : vertices3)
        {
            vertex.position = (vertex.position + v3) * 0.5f;
        }


        std::vector<Vertex> vertices{};
        vertices.insert(vertices.end(), vertices0.begin(), vertices0.end());
        vertices.insert(vertices.end(), vertices1.begin(), vertices1.end());
        vertices.insert(vertices.end(), vertices2.begin(), vertices2.end());
        vertices.insert(vertices.end(), vertices3.begin(), vertices3.end());
        return vertices;
    }
}

MeshRenderData::MeshRenderData(MeshHandle _handle, const glm::mat4& _transform, MaterialHandle _materialHandle, id_t objID)
    : handle(_handle), objectID(objID), transforms{_transform}, materialHandle{_materialHandle}, instanceBuffer{CreateInstanceBuffer(graphicsData->GetBackend().GetDevice(), transforms)} {}
MeshRenderData::MeshRenderData(MeshHandle _handle, const std::vector<glm::mat4>& _transform, MaterialHandle _materialHandle, id_t objID)
    : handle(_handle), objectID(objID), transforms{_transform}, materialHandle{_materialHandle}, instanceBuffer{CreateInstanceBuffer(graphicsData->GetBackend().GetDevice(), transforms)} {}

std::unique_ptr<Buffer> MeshRenderData::CreateInstanceBuffer(internal::Device &device, const std::vector<glm::mat4>& transforms)
{
    uint32_t instanceCount = transforms.size();
    VkDeviceSize bufferSize = sizeof(transforms[0]) * instanceCount;
    uint32_t instanceSize = sizeof(transforms[0]);

    Buffer stagingBuffer{
        device,
        instanceSize,
        instanceCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)transforms.data(), bufferSize);

    std::unique_ptr<Buffer> instanceBuffer = std::make_unique<Buffer>(
        device,
        instanceSize,
        instanceCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    instanceBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
    return std::move(instanceBuffer);
}
} // namespace graphics
