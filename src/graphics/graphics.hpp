#pragma once
#include <memory>

#include "backend/vulkan_backend.hpp"
#include "graphics_data.hpp"

#include "core/mesh.hpp"

namespace graphics
{

// Main graphics class
// Interface for the rest of the engine to use graphics functionality
class Graphics
{
    public:
        Graphics(const std::string& appName, const std::string& engName);
        ~Graphics();

        Graphics(const Graphics&) = delete;
        Graphics& operator=(const Graphics&) = delete;


        void DrawMesh(core::Mesh& meshData, id_t materialID, const glm::mat4& modelMatrix, int instanceID = -1);
        void DrawFrame();


        Window &GetWindow() { return graphicsData->GetWindow(); }
        GLFWwindow* GetGLFWWindow() { return graphicsData->GetWindow().GetWindow(); }

        bool IsOpen() const { return graphicsData->GetWindow().IsOpen(); }
    private:
};

} // namespace graphics