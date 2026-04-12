#include <cstring>

#include <unordered_set>
#include <format>
#include "graphics/backend/vulkan_include.h"

#include "vulkan_backend.hpp"
#include "utils/console.hpp"
#include "utils/debug.hpp"

#include "backend_data.hpp"

#include "graphics/graphics_data.hpp"

namespace graphics::internal
{
// local callback functions
static VKAPI_ATTR uint32_t VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    if(messageSeverity & (vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose))
    {
        Console::log(pCallbackData->pMessage, "Validation Layer");
    }
    else if(messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        Console::warn(pCallbackData->pMessage, "Validation Layer");
    }
    else if(messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
    {
        Console::error(pCallbackData->pMessage, "Validation Layer");
    }

    return VK_FALSE;
}


void VulkanBackend::init(const std::string& appName, const std::string& engName, Window& window)
{
    Console::log("Creating Vulkan Backend", "VulkanBackend");
    applicationName = &appName;
    engineName = &engName;

    // VK_CHECK(volkInitialize());
    VK_CHECK(volkInitialize(), "Failed to initialize Volk");
    Console::log("Initialized Volk");
    createInstance();
    volkLoadInstance(instance);
#ifdef DEBUG
    setupDebugMessenger();
#endif
    window.createSurface(instance);

    features = Features();

    physicalDevice.pickPhysicalDevice(instance, &window.GetSurface());
    createDevice();
    volkLoadDevice(device.Get());
}

VulkanBackend::~VulkanBackend()
{
    Cleanup(graphicsData.get());
}

void VulkanBackend::Cleanup(graphics::GraphicsData *_graphicsData)
{
    if(cleanedUp) return;

    WaitForDevice();

    if(enableValidationLayers)
    {
        vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    if(device.device != VK_NULL_HANDLE)
    {
        device.cleanup();
    }
    if(instance != VK_NULL_HANDLE)
    {
        if(_graphicsData->GetWindow().GetSurface() != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(instance, _graphicsData->GetWindow().GetSurface(), nullptr);

        vkDestroyInstance(instance, nullptr);
    }

    cleanedUp = true;
}

void VulkanBackend::createInstance() 
{
    if (enableValidationLayers && !checkValidationLayerSupport()) 
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    appInfo.pApplicationName = applicationName->c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = engineName->c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    vk::InstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    

    if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess)
        throw std::runtime_error("Failed to create instance!");

    if(!hasGflwRequiredInstanceExtensions())
        throw std::runtime_error("Missing required glfw extension");
}

void VulkanBackend::createDevice()
{
    device.createLogicalDevice(enableValidationLayers);
    device.createCommandPool();
    device.createAllocator(instance);
}

// Debug stuff
VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

void VulkanBackend::setupDebugMessenger() 
{
    if (!enableValidationLayers) return;
    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanBackend::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) 
{
    createInfo.messageSeverity = 
        //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType = 
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

std::vector<const char *> VulkanBackend::getRequiredExtensions() 
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) 
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanBackend::hasGflwRequiredInstanceExtensions() 
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    Console::log("Available extensions:", "VulkanBackend");
    std::unordered_set<std::string> availableExtensions;
    for (const VkExtensionProperties &extension : extensions) 
    {
        Console::log("\t" + std::string(extension.extensionName));
        availableExtensions.insert(extension.extensionName);
    }

    Console::log("Required extensions:", "VulkanBackend");
    auto requiredExtensions = getRequiredExtensions();
    for (const char* &required : requiredExtensions) 
    {
        Console::log("\t" + std::string(required));
        if (availableExtensions.find(required) == availableExtensions.end()) 
        {
            return false;
        }
    }
    return true;
}

bool VulkanBackend::checkValidationLayerSupport() 
{
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char *layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const vk::LayerProperties &layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName.data()) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) 
        {
            return false;
        }
    }

    return true;
}
} // namespace graphics::internal