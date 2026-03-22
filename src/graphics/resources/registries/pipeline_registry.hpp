#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_pipeline.hpp"

namespace graphics
{
    
class PipelineRegistry : public GraphicsRegistry<GraphicsPipeline, PipelineHandle>
{
public:
    PipelineRegistry(internal::Device &device);
    PipelineHandle Register(Shader &shader);
    bool Reload(Shader &shader);
    void ReloadAll();
    using GraphicsRegistry<GraphicsPipeline, PipelineHandle>::Deregister;
    inline bool Deregister(Shader &shader)
    {
        bool result = Deregister(shader);
        return result;
    }
    void Reset();
    void Cleanup();
private:
    internal::Device &device;
    
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    void init();

    void createPipelineCache();
    friend class Graphics;
};

} // namespace graphics