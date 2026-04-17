#include "graphics_mesh.hpp"
#include "graphics/graphics_data.hpp"
#include "utils/packing.hpp"
#include "graphics/utils/packing.hpp"

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

GraphicsMesh::GraphicsMesh(const core::MeshData* mesh) 
    : m_device(graphicsData->GetBackend().GetDevice()),
    m_meshData(mesh)
{
    RecreateBuffers();
}

GraphicsMesh::GraphicsMesh(internal::Device &_device, const core::MeshData* mesh) 
    : m_device(_device),
    m_meshData(mesh)
{
    RecreateBuffers();
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

void GraphicsMesh::Bind(vk::CommandBuffer commandBuffer, const VertexLayout& vertexLayout, const std::unique_ptr<Buffer> &instanceBuffer)
{
    if(!m_vertexBufferIndex.contains(vertexLayout))
        createVertexBuffer(vertexLayout);
    const Buffer& vertexBuffer = m_vertexBuffers[m_vertexBufferIndex.at(vertexLayout)];

    vk::Buffer buffers[] = {m_positionBuffer->GetBuffer(), vertexBuffer.GetBuffer(), instanceBuffer->GetBuffer()};
    vk::DeviceSize offsets[] = {0, 0, 0};
    commandBuffer.bindVertexBuffers(0, 3, buffers, offsets);
    if(m_useIndexBuffer)
        commandBuffer.bindIndexBuffer(m_indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
        
    commandBuffer.setPrimitiveTopology(getVkTopology(m_config.primitiveTopology));
    commandBuffer.setPrimitiveRestartEnable(m_config.enablePrimitiveRestart);
}

void GraphicsMesh::Draw(vk::CommandBuffer commandBuffer, uint32_t instanceCount) const
{
    if(m_useIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, m_indexCount, instanceCount, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, m_vertexCount, instanceCount, 0, 0);
}

void GraphicsMesh::createPositionBuffer()
{
    if(m_meshData == nullptr)
    {
        Console::error("Cannot create vertex buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    const std::vector<Vertex> &vertices = m_meshData->vertices; // Reference to triangles array for easy access

    std::vector<VertexPos> positions{vertices.size()};

    for(uint32_t i = 0; i < vertices.size(); i++)
    {
        const Vertex& vert = vertices[i];
        positions[i].position = vert.position;
    }

    uint32_t positionSize = sizeof(positions[0]);

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        m_device,
        positionSize,
        m_vertexCount
    );

    vk::DeviceSize bufferSize = positionSize * vertices.size();
    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)positions.data(), vertices.size() * positionSize);

    m_positionBuffer = std::make_unique<Buffer>(
        m_device,
        positionSize,
        vertices.size(),
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    m_positionBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
}

using Semantic = VertexLayout::AttrSemantic;
using Format = VertexLayout::AttrFormat;
using Type = VertexLayout::AttrType;

bool directionHelper(void* writeLocation, const VertexLayout::Attribute& attr, glm::vec3 dir)
{
    Format format = attr.format;
    uint32_t formatSize = format.GetSize();
    VertexLayout::AttrType type = format.type;
    if(formatSize == 12 && format.type == Type::Float)
    {
        memcpy(writeLocation, &dir, formatSize);
        return true;
    }
    else if(formatSize == 6 && format.type == Type::Float)
    {
        glm::u16vec3 halfBits = {
            PackFloatToHalf(dir.x),
            PackFloatToHalf(dir.y),
            PackFloatToHalf(dir.z)
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 6 && format.type == Type::SNorm)
    {
        glm::i16vec3 snormVal = {
            dir.x * INT16_MAX,
            dir.y * INT16_MAX,
            dir.z * INT16_MAX
        };
        memcpy(writeLocation, &snormVal, formatSize);
        return true;
    }
    else if(formatSize == 4 && format.type == Type::SNorm)
    {
        glm::i16vec2 octahedral = Packing::PackOctahedral16(dir);
        memcpy(writeLocation, &octahedral, formatSize);
        return true;
    }
    else if(formatSize == 3 && format.type == Type::SNorm)
    {
        glm::i8vec3 snormVal = {
            dir.x * INT8_MAX,
            dir.y * INT8_MAX,
            dir.z * INT8_MAX
        };
        memcpy(writeLocation, &snormVal, formatSize);
        return true;
    }
    else if(formatSize == 2 && format.type == Type::SNorm)
    {
        glm::i8vec2 octahedral = Packing::PackOctahedral8(dir);
        memcpy(writeLocation, &octahedral, formatSize);
        return true;
    }
    return false;
}

bool tangentHelper(void* writeLocation, const VertexLayout::Attribute& attr, glm::vec4 tangent)
{
    uint32_t offset = attr.offset;
    Format format = attr.format;
    uint32_t formatSize = format.GetSize();
    VertexLayout::AttrType type = format.type;
    if(formatSize == 16 && format.type == Type::Float)
    {
        memcpy(writeLocation, &tangent, formatSize);
        return true;
    }
    else if(formatSize == 8 && format.type == Type::Float)
    {
        glm::u16vec4 halfBits = {
            PackFloatToHalf(tangent.x),
            PackFloatToHalf(tangent.y),
            PackFloatToHalf(tangent.z),
            PackFloatToHalf(tangent.w)
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 8 && format.type == Type::SNorm)
    {
        glm::i16vec4 snormVal = {
            tangent.x * INT16_MAX,
            tangent.y * INT16_MAX,
            tangent.z * INT16_MAX,
            tangent.w > 0 ? INT16_MAX : INT16_MIN
        };
        memcpy(writeLocation, &snormVal, formatSize);
        return true;
    }
    else if(formatSize == 6 && format.type == Type::SNorm)
    {
        glm::i16vec3 octahedral = {
            Packing::PackOctahedral16(tangent), 
            tangent.w > 0 ? INT16_MAX : INT16_MIN};
        memcpy(writeLocation, &octahedral, formatSize);
        return true;
    }
    else if(formatSize == 4 && format.type == Type::SNorm)
    {
        glm::i8vec4 snormVal = {
            tangent.x * INT8_MAX,
            tangent.y * INT8_MAX,
            tangent.z * INT8_MAX,
            tangent.w > 0 ? INT8_MAX : INT8_MIN
        };
        memcpy(writeLocation, &snormVal, formatSize);
        return true;
    }
    else if(formatSize == 3 && format.type == Type::SNorm)
    {
        glm::i8vec3 octahedral = {Packing::PackOctahedral8(tangent),
        tangent.w > 0 ? INT8_MAX : INT8_MIN};
        memcpy(writeLocation, &octahedral, formatSize);
        return true;
    }
    return false;
}

bool uvHelper(void* writeLocation, const VertexLayout::Attribute& attr, glm::vec2 uv)
{
    uint32_t offset = attr.offset;
    Format format = attr.format;
    uint32_t formatSize = format.GetSize();
    VertexLayout::AttrType type = format.type;
    if(formatSize == 8 && format.type == Type::Float)
    {
        memcpy(writeLocation, &uv, formatSize);
        return true;
    }
    else if(formatSize == 4 && format.type == Type::Float)
    {
        glm::u16vec2 halfBits = {
            PackFloatToHalf(uv.x),
            PackFloatToHalf(uv.y)
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    return false;
}

bool colorHelper(void* writeLocation, const VertexLayout::Attribute& attr, glm::vec4 color)
{
    uint32_t offset = attr.offset;
    Format format = attr.format;
    uint32_t formatSize = format.GetSize();
    VertexLayout::AttrType type = format.type;
    if(formatSize == 16 && format.type == Type::Float)
    {
        memcpy(writeLocation, &color, formatSize);
        return true;
    }
    else if(formatSize == 12 && format.type == Type::Float)
    {
        glm::vec3 col = glm::vec3(color.r, color.g, color.b);
        memcpy(writeLocation, &col, formatSize);
        return true;
    }
    else if(formatSize == 8 && format.type == Type::Float)
    {
        glm::u16vec4 halfBits = {
            PackFloatToHalf(color.r),
            PackFloatToHalf(color.g),
            PackFloatToHalf(color.b),
            PackFloatToHalf(color.a)
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 6 && format.type == Type::Float)
    {
        glm::u16vec3 halfBits = {
            PackFloatToHalf(color.r),
            PackFloatToHalf(color.g),
            PackFloatToHalf(color.b)
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 8 && format.type == Type::UNorm)
    {
        glm::u16vec4 halfBits = {
            color.r * UINT16_MAX,
            color.g * UINT16_MAX,
            color.b * UINT16_MAX,
            color.a * UINT16_MAX
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 6 && format.type == Type::UNorm)
    {
        glm::u16vec3 halfBits = {
            color.r * UINT16_MAX,
            color.g * UINT16_MAX,
            color.b * UINT16_MAX
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 4 && format.type == Type::UNorm)
    {
        glm::u8vec4 halfBits = {
            color.r * UINT8_MAX,
            color.g * UINT8_MAX,
            color.b * UINT8_MAX,
            color.a * UINT8_MAX
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    else if(formatSize == 3 && format.type == Type::UNorm)
    {
        glm::u8vec3 halfBits = {
            color.r * UINT8_MAX,
            color.g * UINT8_MAX,
            color.b * UINT8_MAX
        };
        memcpy(writeLocation, &halfBits, formatSize);
        return true;
    }
    return false;
}

void GraphicsMesh::createVertexBuffer(const VertexLayout& vertexLayout)
{
    if(m_meshData == nullptr)
    {
        Console::error("Cannot create vertex buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }

    if(m_meshData->vertices.size() != m_vertexCount)
    {
        RecreateBuffers();
    }
    
    const std::vector<Vertex> &vertices = m_meshData->vertices; // Reference to triangles array for easy access

    uint32_t stride = vertexLayout.GetStride();
    uint32_t bufferSize = stride * m_vertexCount;
    std::vector<std::byte> bufferData{bufferSize};

    for(uint32_t i = 0; i < vertices.size(); i++)
    {
        const Vertex& vert = vertices[i];
        for(const VertexLayout::Attribute& attr : vertexLayout.GetAttributes())
        {
            if(attr.semantic == Semantic::Position || attr.semantic == Semantic::I_Model)
                continue;
            
            uint32_t offset = attr.offset;
            Format format = attr.format;
            uint32_t formatSize = format.GetSize();
            VertexLayout::AttrType type = format.type;
            void* writeLocation = bufferData.data() + stride * i + offset;
            switch(attr.semantic)
            {
            case Semantic::Normal:
                // glm::vec3 dir = attr.semantic == Semantic::Normal ? vert.normal : vert.bitangent;
                if(!directionHelper(writeLocation, attr, vert.normal))
                    Console::error("Invalid normal format.", "GraphicsMesh");
                break;
            case Semantic::Tangent:
                if(!tangentHelper(writeLocation, attr, vert.tangent))
                    Console::error("Invalid tangent format.", "GraphicsMesh");
                break;
            case Semantic::UV:
                if(!uvHelper(writeLocation, attr, vert.texCoord))
                    Console::error("Invalid UV format.", "GraphicsMesh");
                break;
            case Semantic::Color:
                if(!colorHelper(writeLocation, attr, vert.color))
                    Console::error("Invalid color format.", "GraphicsMesh");
                break;
            default:
            case Semantic::Other: // TODO: Handle other data
            case Semantic::Bitangent: // Not handled
                break;
            }
        }
    }

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        m_device,
        1,
        bufferSize
    );
    
    stagingBuffer.Map();
    stagingBuffer.WriteData(bufferData.data(), bufferSize);

    uint32_t bufferIndex = m_vertexBuffers.size();
    if(m_vertexBufferIndex.contains(vertexLayout))
    {
        bufferIndex = m_vertexBufferIndex.at(vertexLayout);
        m_vertexBuffers[bufferIndex].~Buffer(); // Recreate buffer in-place
        new (&m_vertexBuffers[bufferIndex]) Buffer(
            m_device,
            1,
            bufferSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );
    }
    else
    {
        m_vertexBufferIndex.insert_or_assign(vertexLayout, bufferIndex);
        m_vertexBuffers.emplace_back(
            m_device,
            1,
            bufferSize,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );
    }
    Buffer& vertexBuffer = m_vertexBuffers[bufferIndex];

    vertexBuffer.CopyFromBuffer(stagingBuffer, bufferSize);
}

void GraphicsMesh::createIndexBuffer()
{
    if(m_meshData == nullptr)
    {
        Console::error("Cannot create index buffer as mesh pointer is null", "GraphicsMesh");
        return;
    }
    
    const std::vector<Triangle> &triangles = m_meshData->triangles; // Reference to triangles array for easy access

    m_indexCount = static_cast<uint32_t>(triangles.size()) * 3;
    m_useIndexBuffer = m_indexCount > 0;

    if(!m_useIndexBuffer)
        return;

    vk::DeviceSize bufferSize = sizeof(triangles[0].v0) * m_indexCount;
    
    uint32_t indexSize = sizeof(triangles[0].v0);

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        m_device,
        indexSize,
        m_indexCount
    );

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)triangles.data());

    m_indexBuffer = std::make_unique<Buffer>(
        m_device,
        indexSize,
        m_indexCount,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    m_indexBuffer->CopyFromBuffer(stagingBuffer, bufferSize);
}
void GraphicsMesh::RecreateBuffers()
{
    m_vertexCount = m_meshData->vertices.size();
    m_indexCount = m_meshData->triangles.size() * 3;
    createPositionBuffer();
    createIndexBuffer();
    for(auto [layout, index] : m_vertexBufferIndex)
    {
        createVertexBuffer(layout);
    }
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

    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        device,
        instanceSize,
        instanceCount
    );

    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)transforms.data(), bufferSize);

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
