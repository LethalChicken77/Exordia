#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"

namespace graphics
{
    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData.backend.Init(appName, engName, graphicsData.GetWindow().GetWindow());
    }
} // namespace graphics