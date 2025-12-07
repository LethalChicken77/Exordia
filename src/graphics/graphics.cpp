#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"
#include "backend/image.hpp"

#include "tests/graphics_tests.hpp"

namespace graphics
{
    // std::unique_ptr<Image> testImage;

    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData = std::make_unique<GraphicsData>();
        graphicsData->window.Init(800, 600, engName + " - " + appName);
        graphicsData->backend.Init(appName, engName, graphicsData->GetGLFWWindow());

        // testImage = std::make_unique<Image>(
        //     graphicsData->GetBackend().GetDevice(),
        //     512,
        //     512,
        //     graphics::ImageProperties::getDefaultProperties(),
        //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        // );
        // testImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        tests::RunAllTests();

        Console::log("Graphics module initialized", "Graphics");
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics