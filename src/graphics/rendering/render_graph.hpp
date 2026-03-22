#pragma once
#include "graphics/backend/image.hpp"
#include "render_context.hpp"

namespace graphics
{

enum AttachmentType
{
    ColorImage,
    DepthImage,
    Buffer
};

struct OutputAttachmentCreateInfo
{
    bool isBuffer;
};

/// @brief Information used to specify the inputs of a render node
struct InputAttachmentCreateInfo
{
    OutputAttachmentCreateInfo* dependency = nullptr; // Image or buffer dependency
    std::string name; // Input name in shader
};

struct RenderNodeCreateInfo
{
    std::vector<InputAttachmentCreateInfo> dependencies;
};

struct InputAttachments
{
    uint32_t nodeIndex;
    uint32_t attachmentIndex;
    bool isBuffer; // Allow use of buffers in addition to textures
};


class RenderNode
{
public:
    std::string name;
    std::vector<InputAttachments> dependencies;

    virtual void Execute(RenderContext context);

private:
    std::vector<Image> imageAttachments{};

    bool useDepthBuffer;
    bool useStencilBuffer;
};

/// @brief A render operation
class RenderPass : public RenderNode
{
public:
    void Execute(RenderContext context);
};

class ComputePass : public RenderNode
{
public:
    void Execute(RenderContext context);
};

class RenderGraph : public RenderNode
{
public:
    class Builder
    {

    };
public:
    void Execute(RenderContext context);
};

};