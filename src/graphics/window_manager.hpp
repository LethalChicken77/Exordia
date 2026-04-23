#pragma once
#include "window.hpp"

namespace graphics
{

class WindowManager
{
public:
    WindowManager();
    ~WindowManager() = default;
    inline void CloseAllWindows() { windows.clear(); }

private:
    std::vector<Window> windows{};
};

} // namespace graphics