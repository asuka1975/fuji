#ifndef FUJI_EXAMPLE_COMMON_VULKAN_ENGINE_HPP
#define FUJI_EXAMPLE_COMMON_VULKAN_ENGINE_HPP

#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

class VulkanEngine {
public:
    VulkanEngine(std::string applicationName);
    void initialize();
    bool isInitialized();

    vk::UniqueInstance& getInstance();
    vk::PhysicalDevice& getPhysicalDevice();
private:
    std::string applicationName;
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;
    bool initialized = false;
protected:
    vk::UniqueInstance createInstance(const std::string& applicationName);
    vk::PhysicalDevice selectPhysicalDevice();
    virtual std::vector<const char*> getRequiredExtensions();
};

#endif