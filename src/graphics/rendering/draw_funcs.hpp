#include <vector>
#include "render_context.hpp"
#include "graphics/resources/graphics_mesh.hpp"

namespace graphics
{
class Graphics;
class GraphicsPipelineOld;
class DrawFunctions
{
    public:
        static void bindCameraDescriptor(const FrameContext &renderContext, VkDescriptorSet descriptorSet, const GraphicsPipelineOld* pipeline);
        static void bindGlobalDescriptor(const FrameContext &renderContext, VkDescriptorSet descriptorSet, const GraphicsPipelineOld* pipeline);
    private:
        static void renderMeshes(FrameContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderGameObjectIDs(FrameContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderFullScreenQuad(FrameContext& frameInfo);

        friend class Graphics;
};
} // namespace graphics