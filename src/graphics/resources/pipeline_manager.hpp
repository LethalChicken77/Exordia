#pragma once
#include <string>
#include <vector>
#include "graphics/backend/vulkan_include.h"

#include "graphics/backend/device.hpp"
#include "graphics_pipeline.hpp"

namespace graphics{

class [[deprecated("Replaced with PipelineRegistry")]] PipelineManager
{
    public:
        PipelineManager(internal::Device &device);
        ~PipelineManager();

        void CreatePipelines();
        void DestroyPipelines();
        void Cleanup();
        void ReloadPipelines();

        void RegisterShader(Shader *shader) { shaders.insert(shader); }
        void DeregisterShader(Shader *shader) { shaders.erase(shader); }

        std::unique_ptr<GraphicsPipelineOld> &GetPipeline(uint32_t index) { return graphicsPipelines[index]; }

    private:
        internal::Device &device;
        std::vector<std::unique_ptr<GraphicsPipelineOld>> graphicsPipelines;

        uint32_t currentID = 0;

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;

        std::set<Shader *> shaders;

        void init();

        void createPipelineCache();
        friend class Graphics;
};
} // namespace graphics