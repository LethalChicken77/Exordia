#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "graphics/backend/device.hpp"
#include "graphics_pipeline.hpp"
#include "core/game_object.hpp"

namespace graphics{

class PipelineManager
{
    public:
        PipelineManager(internal::Device &device);
        ~PipelineManager();

        void CreatePipelines();
        void DestroyPipelines();
        void ReloadPipelines();

        void RegisterShader(core::Shader *shader) { shaders.insert(shader); }
        void DeregisterShader(core::Shader *shader) { shaders.erase(shader); }

        std::unique_ptr<GraphicsPipeline> &GetPipeline(uint32_t index) { return graphicsPipelines[index]; }

    private:
        internal::Device &device;
        std::vector<std::unique_ptr<GraphicsPipeline>> graphicsPipelines;

        uint32_t currentID = 0;

        VkPipelineCache pipelineCache;

        std::set<core::Shader *> shaders;

        void init();

        void createPipelineCache();
        friend class Graphics;
};
} // namespace graphics