#ifndef FUJI_EXAMPLE_COMMON_VULKAN_ENGINE_HPP
#define FUJI_EXAMPLE_COMMON_VULKAN_ENGINE_HPP

#include <functional>
#include <string>
#include <optional>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NDEBUG 1

struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanEngine {
public:
    VulkanEngine(std::string applicationName);
    void initialize(GLFWwindow* window);
    void setupRenderTarget(vk::RenderPass renderPass);
    void setCommandBufferRecorder(std::function<void(vk::Framebuffer&)> recorder);
    bool isInitialized();
    void drawFrame(vk::CommandBuffer& commandBuffer);

    vk::UniqueInstance& getInstance();
    vk::PhysicalDevice& getPhysicalDevice();
    vk::UniqueDevice& getDevice();
    vk::UniqueSurfaceKHR& getSurface();
    vk::UniqueCommandPool& getCommandPool();
    const vk::Format& getFormat() const;
    const vk::Extent2D& getSwapchainExtent() const;
    vk::Queue& getGraphicsQueue();

    vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);
private:
    std::string applicationName;
    vk::UniqueInstance instance;
    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::UniqueSurfaceKHR surface;
    vk::UniqueSwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;
    std::vector<vk::UniqueImageView> swapchainImageViews;
    std::vector<vk::UniqueFramebuffer> framebuffers;
    vk::UniqueCommandPool commandPool;
    vk::UniqueSemaphore imageAvailableSemaphore;
    vk::UniqueSemaphore renderFinishedSemaphore;
    vk::UniqueFence inFlightFence;
    std::function<void(vk::Framebuffer&)> recorder;
    GLFWwindow* window;
    bool initialized = false;

    vk::UniqueDebugUtilsMessengerEXT debugMessenger;
protected:
    vk::UniqueInstance createInstance(const std::string& applicationName);
    vk::UniqueDevice createDevice();
    vk::UniqueSurfaceKHR createSurface();
    vk::UniqueSwapchainKHR createSwapchain();
    std::vector<vk::UniqueImageView> createImageViews();
    vk::UniqueDebugUtilsMessengerEXT createDebugMessenger();
    std::vector<vk::UniqueFramebuffer> createFramebuffers(vk::RenderPass renderPass);
    vk::UniqueSemaphore createImageAvailableSemaphore();
    vk::UniqueSemaphore createRenderFinishedSemaphore();
    vk::UniqueFence createInFlightFence();
    vk::UniqueCommandPool createCommandPool();
    vk::PhysicalDevice choosePhysicalDevice();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    std::optional<std::pair<vk::QueueFamilyProperties, std::uint32_t>> findGraphicsQueueFamility(vk::PhysicalDevice& physicalDevice);
    std::optional<std::pair<vk::QueueFamilyProperties, std::uint32_t>> findPresentQueueFamility(vk::PhysicalDevice& physicalDevice);
    bool checkValidationLayerSupport();
    bool isDeviceSuitable(vk::PhysicalDevice& physicalDevice);
    SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice& physicalDevice);
    virtual std::vector<std::string> getRequiredValidationLayers();
    virtual std::vector<std::string> getRequiredExtensions();
    virtual std::vector<std::string> getRequiredDeviceExtensions();
};

#endif