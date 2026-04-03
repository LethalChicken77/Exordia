#pragma once
#include "graphics/resources/texture.hpp"
#include "graphics/resources/buffer.hpp"
#include "render_context.hpp"

namespace graphics
{

struct InputImage
{
    uint32_t set;
    uint32_t binding;
    Image* image;
};

struct InputBuffer
{
    uint32_t set;
    uint32_t binding;
    Buffer* image;
};

struct RenderResult
{
    std::vector<Image*> images;
    std::vector<Buffer*> buffers;
};

/// @brief An abstract render step
class RenderNode
{
public:
    virtual void Execute(const RenderContext& context, Image* pResult, Image* pDepthResult) = 0;
    virtual void Execute(const RenderContext& context, RenderResult* pResult) = 0;

    void AddImage(Image* pImage, uint32_t set, uint32_t binding) noexcept;
    void AddBuffer(Buffer* pBuffer, uint32_t set, uint32_t binding) noexcept;
private:
    std::vector<InputImage> inputImages;
    std::vector<InputBuffer> inputBuffers;
};

/// @brief Takes buffers and images as input, returns an image as output
class RenderPass : public RenderNode
{
public:
    void Execute(const RenderContext& context, Image* pResult, Image* pDepthResult) override;
    void Execute(const RenderContext& context, RenderResult* pResult) override;
private:
    void bindInputs();
};

/// @brief A set of render passes and their data
/// @note Only owns internal images and buffers. Inputs and outputs must be passed.
class RenderStage : public RenderNode
{
public:
    RenderStage(uint32_t maxResources = 32);
    void Execute(const RenderContext& context, Image* pResult, Image* pDepthResult) override;
    void Execute(const RenderContext& context, RenderResult* pResult) override;
private:
    std::vector<Image> images{};
    std::vector<Buffer> buffers{};
};

} // namespace graphics