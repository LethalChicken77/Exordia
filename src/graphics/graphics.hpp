#pragma once
#include "backend/vulkan_backend.hpp"

namespace graphics
{

class Graphics
{
    public:
        Graphics() = default;
        ~Graphics();

        Graphics(const Graphics&) = delete;
        Graphics& operator=(const Graphics&) = delete;
    private:
        internal::VulkanBackend backend;
};

} // namespace graphics