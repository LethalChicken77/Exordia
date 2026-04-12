#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_pipeline.hpp"

namespace graphics
{

/// @brief Manager for graphics pipelines. Shaders are registered, and a handle is returned.
class PipelineRegistry : public GraphicsRegistry<GraphicsPipelineOld, PipelineHandle>
{
public:
    PipelineRegistry(internal::Device &device);
    ~PipelineRegistry();
    
    PipelineHandle Register(Shader &shader);
    bool Reload(Shader &shader);
    void ReloadAll();
    using GraphicsRegistry<GraphicsPipelineOld, PipelineHandle>::Deregister;
    inline bool Deregister(Shader &shader)
    {
        return Deregister(shader);
    }
    void Cleanup();
private:
    internal::Device &device;
    
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    void init();

    void createPipelineCache();
    friend class Graphics;
};

} // namespace graphics