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
        static void bindCameraDescriptor(const RenderContext &renderContext, VkDescriptorSet descriptorSet, GraphicsPipeline* pipeline);
        static void bindGlobalDescriptor(const RenderContext &renderContext, VkDescriptorSet descriptorSet, GraphicsPipeline* pipeline);
    private:
        static void renderMeshes(RenderContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderGameObjectIDs(RenderContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderFullScreenQuad(RenderContext& frameInfo);

        friend class Graphics;
};
} // namespace graphics