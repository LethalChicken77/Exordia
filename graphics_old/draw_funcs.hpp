#include <vector>
#include "render_context.hpp"
#include "graphics_mesh.hpp"

namespace graphics
{
class Graphics;
class DrawFunctions
{
    private:
        static void renderMeshes(RenderContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderGameObjectIDs(RenderContext& frameInfo, const std::vector<MeshRenderData> &renderQueue);
        static void renderFullScreenQuad(RenderContext& frameInfo);
        static void bindCameraDescriptor(RenderContext& frameInfo, GraphicsPipeline* pipeline);
        static void bindGlobalDescriptor(RenderContext& frameInfo, GraphicsPipeline* pipeline);

        friend class Graphics;
};
} // namespace graphics