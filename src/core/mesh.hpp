#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "utils/console.hpp"
#include "utils/smart_reference.hpp"
#include "ref.hpp"
#include "graphics/api/handles.hpp"
#include "graphics/api/mesh_config.hpp"
#include "primitives/color.hpp"

namespace core
{
    
    struct MeshImportOptions
    {
        glm::vec3 importScale = glm::vec3(1);
    };

    class MeshData : RefCounted
    {
    public:
        static constexpr const char* className = "Mesh Data";
        const char* GetClassName() const { return className; }
        
        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec4 tangent{};
            // glm::vec3 bitangent{};
            Color color{1.0, 1.0, 1.0, 1.0};
            glm::vec2 texCoord{};
        };
        struct VertexPos
        {
            alignas(4) glm::vec3 position{};
        };
        struct VertexNoPos
        {
            glm::vec3 normal{}; // 12
            glm::vec4 tangent{}; // 16
            // glm::vec3 bitangent{}; // 12
            glm::i8vec3 color{255, 255, 255}; // 3
            glm::vec2 texCoord{}; // 8
            // 72
        };

        struct SmallVertex
        {
            glm::vec3 position{}; // 12
            glm::i8vec2 normal{}; // 2
            glm::i8vec4 tangent{}; // 4
            glm::i8vec3 color{1.0f, 1.0f, 1.0f}; // 6
            glm::vec2 texCoord{}; // 8
            // 32
        };

        struct alignas(16) TinyVertex
        {
            glm::vec3 position{}; // 12
            glm::i8vec2 normal{}; // 2
            glm::i16vec2 texCoord{}; // 4 (float 16 data, not int)
            // 18
        };

        struct alignas(16) MinimalVertex
        {
            glm::vec3 position{}; // 12
            glm::i8vec2 normal{}; // 2
            // 14
        };

        struct Triangle
        {
            uint32_t v0;
            uint32_t v1;
            uint32_t v2;
        };

        std::string name = "NewMesh";
        
        std::vector<Vertex> vertices{};
        std::vector<Triangle> triangles{};

        graphics::MeshConfig config{};

        graphics::MeshHandle graphicsHandle{};

        MeshData() = default;
        ~MeshData();

        MeshData(const MeshData&) = delete;
        MeshData(MeshData&&);

        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _indices);
        void UpdateOnGPU();

    private:
        void loadModelFromObj(const std::string& filename); // TODO: Asset importer
    };

    class Mesh
    {
        public:
            using Vertex = MeshData::Vertex;
            using Triangle = MeshData::Triangle;

            Mesh() = default;
            Mesh(std::vector<Vertex> &vertices, const std::string& objectName = "New Mesh");
            Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, const std::string& objectName = "New Mesh");
            Mesh(std::vector<Vertex> &vertices, std::vector<Triangle> &triangles, const std::string& objectName = "New Mesh");

            void generateNormals();
            void generateTangents();

            void PrintInfo() const
            {
                if(meshData) 
                {
                    Console::log("Mesh " + meshData->name + " has " + std::to_string(meshData->vertices.size()) + " vertices and " + std::to_string(meshData->triangles.size()) + " triangles.", "Mesh");
                }
            }

            // Mesh creation
            static Mesh createCube(float edgeLength, const std::string& objectName = "Cube Mesh");
            static Mesh createSierpinskiPyramid(float edgeLength, int depth, const std::string& objectName = "Sierpinski Pyramid Mesh");
            static Mesh createGrid(int width, int length, glm::vec2 dimensions, const std::string& objectName = "Grid Mesh");
            static Mesh createSkybox(float size, const std::string& objectName = "Skybox Mesh");
            static Mesh loadObj(const std::string& filename, const std::string& objectName = "Obj Mesh", MeshImportOptions importOptions = {}); // TODO: replace with loadFromFile
            
            explicit operator bool() const noexcept { return !meshData.IsNull(); }
            MeshData& operator*() const noexcept { return *meshData; }
            MeshData* operator->() const noexcept { return meshData.Get(); }
            bool operator==(const Mesh& other) const { return meshData == other.meshData; }
            bool operator!=(const Mesh& other) const { return meshData != other.meshData; }
        private:
            Ref<MeshData> meshData{};
            void loadModelFromObj(const std::string& filename);
    };
}