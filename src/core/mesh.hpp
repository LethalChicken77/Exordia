#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "object_manager.hpp"
#include "utils/console.hpp"
#include "utils/smart_reference.hpp"
#include "graphics/api/handles.hpp"

namespace core
{
    class MeshData : public Object // TODO: Redesign to avoid inheritance
    {
    public:
        static constexpr const char* className = "Mesh Data";
        const char* GetClassName() const { return className; }
        
        struct alignas(16) Vertex
        {
            glm::vec3 position{}; // 12
            glm::vec3 normal{}; // 12
            glm::vec4 tangent{}; // 16
            glm::vec3 bitangent{}; // 12
            glm::vec3 color{1.0f, 1.0f, 1.0f}; // 12
            glm::vec2 texCoord{}; // 8
            // 72
        };

        struct alignas(16) SmallVertex
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

        // TODO: RAII, why not destroy and recreate the object if the vectors are reallocated anyway?

        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
        void SetMesh(const std::vector<Vertex>& _vertices, const std::vector<Triangle>& _indices);
        ~MeshData();

        std::vector<Vertex> vertices{};
        std::vector<Triangle> triangles{};

        graphics::MeshHandle graphicsHandle{};

    private:
        using Object::Object;
        void loadModelFromObj(const std::string& filename); // TODO: Asset importer
    };

    class Mesh : public SmartRef<MeshData> // TODO: Delete smartref because it is not smart and it is not useful
    {
        public:
            using SmartRef<MeshData>::SmartRef; // Inherit base constructor

            using Vertex = MeshData::Vertex;
            using Triangle = MeshData::Triangle;

            Mesh(std::vector<Vertex> &vertices, const std::string& objectName = "New Mesh");
            Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, const std::string& objectName = "New Mesh");
            Mesh(std::vector<Vertex> &vertices, std::vector<Triangle> &triangles, const std::string& objectName = "New Mesh");

            void generateNormals();
            void generateTangents();

            void PrintInfo() const
            {
                if(ptr) 
                {
                    Console::log("Mesh " + ptr->name + " has " + std::to_string(ptr->vertices.size()) + " vertices and " + std::to_string(ptr->triangles.size()) + " triangles.", "Mesh");
                }
            }

            // Mesh creation
            static Mesh createCube(float edgeLength, const std::string& objectName = "Cube Mesh");
            static Mesh createSierpinskiPyramid(float edgeLength, int depth, const std::string& objectName = "Sierpinski Pyramid Mesh");
            static Mesh createGrid(int width, int length, glm::vec2 dimensions, const std::string& objectName = "Grid Mesh");
            static Mesh createSkybox(float size, const std::string& objectName = "Skybox Mesh");
            static Mesh loadObj(const std::string& filename, const std::string& objectName = "Obj Mesh"); // TODO: replace with loadFromFile
        private:
            void loadModelFromObj(const std::string& filename);
    };
}