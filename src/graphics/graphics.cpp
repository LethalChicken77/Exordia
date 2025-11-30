#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"

namespace graphics
{
    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData.window.Init(800, 600, engName + " - " + appName);
        graphicsData.backend.Init(appName, engName, graphicsData.GetGLFWWindow());
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics