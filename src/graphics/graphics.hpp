#pragma once
#include <memory>

#include "backend/vulkan_backend.hpp"
#include "graphics_data.hpp"
#include "resources/graphics_mesh.hpp"

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


        void DrawMesh(const core::Mesh& meshData, id_t materialID, const glm::mat4& modelMatrix, int instanceID = -1);
        void DrawFrame();

        void RegisterShader(Shader *shader, bool reloadPipelines = true)
        {
            graphicsData->pipelineManager.RegisterShader(shader);
            if(reloadPipelines)
                graphicsData->pipelineManager.ReloadPipelines();
        }
        
        void DeregisterShader(Shader *shader, bool reloadPipelines = true)
        {
            graphicsData->pipelineManager.DeregisterShader(shader);
            if(reloadPipelines)
                graphicsData->pipelineManager.ReloadPipelines();
        }
        
        inline void RegisterMaterial(Material *mat, bool reloadPipelines = true)
        {
            graphicsData->testMaterial = std::make_unique<ShaderBuffer>(mat);
        }
        
        inline void DeregisterMaterial(Material *mat, bool reloadPipelines = true)
        {
            graphicsData->testMaterial.reset();
        }

        inline MeshHandle RegisterMesh(core::MeshData &mesh) { return graphicsData->meshManager.RegisterMesh(mesh); }
        inline bool UpdateMesh(core::MeshData &mesh) { return graphicsData->meshManager.UpdateMesh(mesh); }
        inline bool DeregisterMesh(core::MeshData &mesh) { return graphicsData->meshManager.DeregisterMesh(mesh); }

        inline void ReloadPipelines()
        {
            graphicsData->pipelineManager.ReloadPipelines();
        }

        Window &GetWindow() { return graphicsData->GetWindow(); }
        GLFWwindow* GetGLFWWindow() { return graphicsData->GetWindow().GetWindow(); }

        bool IsOpen() const { return graphicsData->GetWindow().IsOpen(); }
        inline internal::Device &GetDevice() { return graphicsData->GetBackend().GetDevice(); }
        void GraphicsInitImgui();
    private:
        std::vector<MeshRenderData> drawQueue;
        // Temporary
        void drawImgui(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

} // namespace graphics