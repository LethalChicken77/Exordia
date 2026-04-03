#include "render_node.hpp"

namespace graphics
{


void RenderNode::AddImage(Image* pImage, uint32_t set, uint32_t binding) noexcept
{
    inputImages.push_back({set, binding, pImage});
}

void RenderNode::AddBuffer(Buffer* pBuffer, uint32_t set, uint32_t binding) noexcept
{
    inputBuffers.push_back({set, binding, pBuffer});
}

void RenderPass::Execute(const RenderContext& context, Image* pResult, Image* pDepthResult)
{

}

void RenderPass::Execute(const RenderContext& context, RenderResult* pResult)
{

}

RenderStage::RenderStage(uint32_t maxResources)
{
    images.reserve(maxResources);
    buffers.reserve(maxResources);
}

void RenderStage::Execute(const RenderContext& context, Image* pResult, Image* pDepthResult)
{

}

void RenderStage::Execute(const RenderContext& context, RenderResult* pResult)
{

}

};