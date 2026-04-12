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

void RenderPass::Execute(const FrameContext& context, Image* pResult, Image* pDepthResult)
{

}

void RenderPass::Execute(const FrameContext& context, RenderResult* pResult)
{

}

RenderStage::RenderStage(uint32_t maxResources)
{
    images.reserve(maxResources);
    buffers.reserve(maxResources);
}

void RenderStage::Execute(const FrameContext& context, Image* pResult, Image* pDepthResult)
{

}

void RenderStage::Execute(const FrameContext& context, RenderResult* pResult)
{

}

};