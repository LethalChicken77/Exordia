#pragma once

namespace graphics::internal
{
class SwapChain
{
public:
    SwapChain() = default;
    ~SwapChain() = default;

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

private:
    
};
} // namespace graphics::internal