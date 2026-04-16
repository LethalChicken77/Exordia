#include <vector>
#include "render_context.hpp"
#include "graphics/resources/graphics_mesh.hpp"

namespace graphics
{
class Graphics;
class GraphicsPipeline;
class DrawFunctions
{
    public:
        static void bindCameraDescriptor(const FrameContext &renderContext, vk::DescriptorSet descriptorSet, const GraphicsPipeline* pipeline);
        static void bindGlobalDescriptor(const FrameContext &renderContext, vk::DescriptorSet descriptorSet, const GraphicsPipeline* pipeline);
    private:
        static void renderMeshes(FrameContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderGameObjectIDs(FrameContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderFullScreenQuad(FrameContext& frameInfo);

        friend class Graphics;
};
} // namespace graphics