#pragma once
#include <memory>

#include "backend/vulkan_backend.hpp"
#include "graphics_data.hpp"
#include "resources/graphics_mesh.hpp"
#include "primitives/camera.hpp"

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


        void DrawMesh(const core::Mesh& meshData, id_t materialID, const std::vector<glm::mat4>& modelMatrices, int instanceID = -1);
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
        
        inline void RegisterMaterial(Material &mat)
        {
            // const DescriptorSetLayout &layout, DescriptorPool &pool, uint32_t binding, const std::vector<uint8_t> data
            // graphicsData->testMaterial = std::make_unique<GraphicsMaterial>(
            //     graphicsData->pipelineManager.GetPipeline(0)->GetDescriptorSetLayout(),
            //     *graphicsData->materialDescriptorPool,
            //     0,
            //     mat->GetData()
            // );
            // if(mat == nullptr) return;
            // graphicsData->materialRegistry.Register(*mat);
        }
        
        inline void DeregisterMaterial(Material &mat)
        {
            graphicsData->testMaterial.reset();
        }

        inline MeshHandle RegisterMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Register(mesh); }
        inline bool UpdateMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Update(mesh); }
        inline bool DeregisterMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Deregister(mesh); }
        
        void SetCamera(const Camera &camera);

        inline void ReloadPipelines()
        {
            graphicsData->pipelineManager.ReloadPipelines();
        }

        Window &GetWindow() { return graphicsData->GetWindow(); }
        GLFWwindow* GetGLFWWindow() { return graphicsData->GetWindow().GetWindow(); }
        float GetAspectRatio() const { return graphicsData->GetWindow().GetAspectRatio(); }

        bool IsOpen() const { return graphicsData->GetWindow().IsOpen(); }
        inline internal::Device &GetDevice() { return graphicsData->GetBackend().GetDevice(); }
        void GraphicsInitImgui();
    private:
        std::vector<MeshRenderData> drawQueue;
        CameraUbo cameraState{};
        // Temporary
        void drawImgui(RenderContext context);
};

} // namespace graphics