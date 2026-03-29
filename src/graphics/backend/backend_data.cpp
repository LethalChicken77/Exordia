#include "backend_data.hpp"
namespace graphics::internal
{
Features::Features()
{
    // Enable/disable features here
    // Features will be validated and enabled
    
    vk::PhysicalDeviceFeatures2 &features2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
    vk::PhysicalDeviceVulkan11Features &features11 = featureChain.get<vk::PhysicalDeviceVulkan11Features>();
    vk::PhysicalDeviceVulkan12Features &features12 = featureChain.get<vk::PhysicalDeviceVulkan12Features>();
    vk::PhysicalDeviceVulkan13Features &features13 = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
    vk::PhysicalDeviceVulkan14Features &features14 = featureChain.get<vk::PhysicalDeviceVulkan14Features>();

    features2.features.samplerAnisotropy = VK_TRUE;
    features2.features.fillModeNonSolid = VK_TRUE;
    features2.features.shaderFloat64 = VK_TRUE;
    
    features11.uniformAndStorageBuffer16BitAccess = VK_TRUE;
    features12.shaderFloat16 = VK_TRUE;
    features12.uniformAndStorageBuffer8BitAccess = VK_TRUE;
    features12.shaderInt8 = VK_TRUE;
    
    features13.dynamicRendering = VK_TRUE;
    
    features12.scalarBlockLayout = VK_TRUE;

}

bool Features::operator==(const Features &other) const
{
    return other.featureChain == this->featureChain;
}

Features features{};
}