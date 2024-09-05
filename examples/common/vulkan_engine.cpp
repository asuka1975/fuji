#include "vulkan_engine.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_enums.hpp>

VulkanEngine::VulkanEngine(std::string applicationName) : applicationName(std::move(applicationName)) {

}

void VulkanEngine::initialize() {
    this->instance = createInstance(applicationName);

    std::cout << "**instance extensions**: " << std::endl;
    auto extensions = vk::enumerateInstanceExtensionProperties();
    for(auto extention : extensions) {
        std::cout << extention.extensionName << " " << extention.specVersion << std::endl;
    }
    this->physicalDevice = selectPhysicalDevice();

    initialized = true;
}

bool VulkanEngine::isInitialized() {
    return initialized;
}

vk::UniqueInstance& VulkanEngine::getInstance() {
    return this->instance;
}

vk::PhysicalDevice& VulkanEngine::getPhysicalDevice() {
    return this->physicalDevice;
}

vk::UniqueInstance VulkanEngine::createInstance(const std::string& applicationName) {
    vk::ApplicationInfo applicationInfo {
        applicationName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3
    };

    auto extensions = getRequiredExtensions();

    vk::InstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    return vk::createInstanceUnique(instanceCreateInfo);
}

vk::PhysicalDevice VulkanEngine::selectPhysicalDevice() {
    auto physicalDevices = this->instance->enumeratePhysicalDevices();

    for(auto physicalDevice : physicalDevices) {
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        if(std::find_if(queueFamilies.begin(), queueFamilies.end(), [](auto queueFamily) { return queueFamily.queueFlags & vk::QueueFlagBits::eGraphics; }) != queueFamilies.end()) {
            return physicalDevice;
        }
    }

    throw std::runtime_error("no suitable PhysicalDevice");
}

std::vector<const char*> VulkanEngine::getRequiredExtensions() {
    std::vector<const char*> extensions;

    std::uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

    return extensions;
}