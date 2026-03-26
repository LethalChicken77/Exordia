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


        void DrawMesh(const core::Mesh& meshData, const Material& material, const std::vector<glm::mat4>& modelMatrices, int instanceID = -1);
        void DrawMesh(const core::Mesh& meshData, const Material& material, const glm::mat4& modelMatrix, int instanceID = -1);
        void DrawFrame();

        inline void RegisterShader(Shader &shader) { graphicsData->pipelineRegistry.Register(shader); }
        inline void DeregisterShader(Shader &shader) { graphicsData->pipelineRegistry.Deregister(shader); }

        void RegisterMaterial(Material &mat) 
        { 
            graphicsData->materialRegistry.Register(mat, *graphicsData->materialDescriptorPool); // TODO: Deal with descriptor pools differently
        }
        inline void DeregisterMaterial(Material &mat) { graphicsData->materialRegistry.Deregister(mat); }

        inline MeshHandle RegisterMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Register(mesh); }
        inline bool UpdateMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Update(mesh); }
        inline bool DeregisterMesh(core::MeshData &mesh) { return graphicsData->meshRegistry.Deregister(mesh); }

        inline TextureHandle RegisterTexture(TextureData &tex) { return graphicsData->textureRegistry.Register(tex); }
        inline bool UpdateTexture(TextureData &tex) { return graphicsData->textureRegistry.Update(tex); }
        inline bool DeregisterTexture(TextureData &tex) { return graphicsData->textureRegistry.Deregister(tex); }
        
        void SetCamera(const Camera &camera);

        // inline void ReloadPipelines()
        // {
        //     graphicsData->pipelineRegistry.ReloadPipelines();
        // }

        Window &GetWindow() { return graphicsData->GetWindow(); }
        GLFWwindow* GetGLFWWindow() { return graphicsData->GetWindow().GetWindow(); }
        float GetAspectRatio() const { return graphicsData->GetWindow().GetAspectRatio(); }

        bool IsOpen() const { return graphicsData->GetWindow().IsOpen(); }
        inline internal::Device &GetDevice() { return graphicsData->GetBackend().GetDevice(); }
        void GraphicsInitImgui();
    private:
        std::vector<MeshRenderData> drawQueue{};
        CameraUbo cameraState{};
        // Temporary
        void drawImgui(RenderContext context);
};

} // namespace graphics