#include <stdexcept>

#include "device.hpp"

namespace graphics::internal
{

PhysicalDevice::PhysicalDevice() 
{
    pickPhysicalDevice();
}

} // namespace graphics::internal