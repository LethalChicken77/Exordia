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

    features2.features.samplerAnisotropy = true;
    features2.features.fillModeNonSolid = true;
    features2.features.shaderFloat64 = true;
    
    features11.uniformAndStorageBuffer16BitAccess = true;
    features12.shaderFloat16 = true;
    features12.uniformAndStorageBuffer8BitAccess = true;
    features12.shaderInt8 = true;
    
    features13.dynamicRendering = true;
    
    features12.scalarBlockLayout = true;
    
    features13.maintenance4 = true;
    features14.maintenance5 = true;
    features14.maintenance6 = true;
}

bool Features::operator==(const Features &other) const
{
    return other.featureChain == this->featureChain;
}

Features features{};
}