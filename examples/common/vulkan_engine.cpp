#include "vulkan_engine.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
    ) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
}


VulkanEngine::VulkanEngine(std::string applicationName) : applicationName(std::move(applicationName)) {

}

void VulkanEngine::initialize(GLFWwindow* window) {
    this->window = window;

#ifdef NDEBUG
    std::cout << "**validation layers**: " << std::endl;
    auto layers = vk::enumerateInstanceLayerProperties();
    for(auto& layer : layers) {
        std::cout << "  " << layer.layerName << std::endl;
    }
#endif
    this->instance = createInstance(applicationName);

#ifdef NDEBUG
    this->debugMessenger = createDebugMessenger();
#endif

    this->surface = createSurface();

    std::cout << "**instance extensions**: " << std::endl;
    auto extensions = vk::enumerateInstanceExtensionProperties();
    for(auto extention : extensions) {
        std::cout << "  " << extention.extensionName << " " << extention.specVersion << std::endl;
    }
    this->physicalDevice = choosePhysicalDevice();
    std::cout << "**device extensions**: " << std::endl;
    auto deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    for(auto extension : deviceExtensions) {
        std::cout << "  " << extension.extensionName << " " << extension.specVersion << std::endl;
    }

    this->device = createDevice();
    auto [graphicsQueueFamily, graphicsQueueIndex] = findGraphicsQueueFamility(this->physicalDevice).value();
    this->graphicsQueue = this->device->getQueue(graphicsQueueIndex, 0);
    auto [presentQueueFamily, presentQueueIndex] = findPresentQueueFamility(this->physicalDevice).value();
    this->presentQueue = this->device->getQueue(presentQueueIndex, 0);
    this->swapchain = this->createSwapchain();
    this->swapchainImageViews = this->createImageViews();

    this->imageAvailableSemaphore = this->createImageAvailableSemaphore();
    this->renderFinishedSemaphore = this->createRenderFinishedSemaphore();
    this->inFlightFence = this->createInFlightFence();

    this->commandPool = this->createCommandPool();

    this->initialized = true;
}

void VulkanEngine::drawFrame(vk::CommandBuffer& commandBuffer) {
    auto result = this->device->waitForFences({ this->inFlightFence.get() }, VK_TRUE, 5000000000L);
    if(result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait fence");
    }
    this->device->resetFences({ this->inFlightFence.get() });

    auto [result1, imageIndex] = this->device->acquireNextImageKHR(swapchain.get(), UINT64_MAX, this->imageAvailableSemaphore.get(), VK_NULL_HANDLE);
    if(result1 != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire next image");
    }

    commandBuffer.reset();
    recorder(framebuffers[imageIndex].get());

    vk::SubmitInfo submitInfo {};

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore.get() };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore.get() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    graphicsQueue.submit({ submitInfo }, inFlightFence.get());

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapchains[] = { swapchain.get() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    auto result2 = presentQueue.presentKHR(presentInfo);
    if(result2 != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present queue");
    }
}

void VulkanEngine::setupRenderTarget(vk::RenderPass renderPass) {
    this->framebuffers = this->createFramebuffers(renderPass);
}

void VulkanEngine::setCommandBufferRecorder(std::function<void (vk::Framebuffer&)> recorder) {
    this->recorder = recorder;
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

vk::UniqueDevice& VulkanEngine::getDevice() {
    return this->device;
}

vk::UniqueSurfaceKHR& VulkanEngine::getSurface() {
    return this->surface;
}

vk::UniqueCommandPool& VulkanEngine::getCommandPool() {
    return this->commandPool;
}

const vk::Format& VulkanEngine::getFormat() const {
    return swapchainImageFormat;
}

const vk::Extent2D& VulkanEngine::getSwapchainExtent() const {
    return swapchainExtent;
}

vk::Queue& VulkanEngine::getGraphicsQueue() {
    return this->graphicsQueue;
}

vk::UniqueShaderModule VulkanEngine::createShaderModule(const std::vector<char> &code) {
    vk::ShaderModuleCreateInfo createInfo {};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

    return this->device->createShaderModuleUnique(createInfo);
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
    std::vector<const char*> cstrExtensions;
    std::transform(extensions.begin(), extensions.end(), std::back_inserter(cstrExtensions), [](auto& s) { return s.c_str(); });

    vk::InstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(cstrExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = cstrExtensions.data();


#ifdef NDEBUG
    auto requiredValidationLayers = getRequiredValidationLayers();
    std::vector<const char*> requiredCstrValidationLayers;
    std::transform(requiredValidationLayers.begin(), requiredValidationLayers.end(), std::back_inserter(requiredCstrValidationLayers), [](auto& layer) { return layer.c_str(); });
    instanceCreateInfo.enabledLayerCount = requiredCstrValidationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = requiredCstrValidationLayers.data();
#else
    instanceCreateInfo.enabledLayerCount = 0;
#endif

    return vk::createInstanceUnique(instanceCreateInfo);
}

vk::UniqueDevice VulkanEngine::createDevice() {
    assert(this->physicalDevice != static_cast<vk::PhysicalDevice>(nullptr));

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = {
        findGraphicsQueueFamility(this->physicalDevice).value().second,
        findPresentQueueFamility(this->physicalDevice).value().second,
    };

    float queuePriority = 1.0f;
    for(auto queueIndex : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.queueFamilyIndex = queueIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures physicalDeviceFeatures;

    auto extensions = this->getRequiredDeviceExtensions();
    std::vector<const char*> cstrExtensions;
    std::transform(extensions.begin(), extensions.end(), std::back_inserter(cstrExtensions), [](auto& s) { return s.c_str(); });
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.ppEnabledExtensionNames = cstrExtensions.data();
    deviceCreateInfo.enabledExtensionCount = cstrExtensions.size();
#ifdef NDEBUG
    auto layers = getRequiredValidationLayers();
    std::vector<const char*> cstrLayers;
    std::transform(layers.begin(), layers.end(), std::back_inserter(cstrLayers), [](auto& layer) { return layer.c_str(); });

    deviceCreateInfo.enabledLayerCount = cstrLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = cstrLayers.data();
#endif

    return this->physicalDevice.createDeviceUnique(deviceCreateInfo);
}

vk::UniqueSurfaceKHR VulkanEngine::createSurface() {
    assert(this->instance.get() != static_cast<vk::Instance>(nullptr) && this->window != nullptr);

    return vk::UniqueSurfaceKHR([&]() {
        VkSurfaceKHR surface;
        if(auto err = glfwCreateWindowSurface(*this->instance, this->window, nullptr, &surface); err != VK_SUCCESS) {
            std::cerr << err << std::endl;
            throw std::runtime_error("failed to create Surface");
        }

        return surface;
    }(), *this->instance);
}

vk::UniqueSwapchainKHR VulkanEngine::createSwapchain() {
    assert(this->physicalDevice != static_cast<vk::PhysicalDevice>(nullptr) && this->device.get() != static_cast<vk::Device>(nullptr) && this->surface.get() != static_cast<vk::SurfaceKHR>(nullptr));

    SwapchainSupportDetails details = querySwapchainSupport(this->physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
    vk::Extent2D extent = chooseSwapExtent(details.capabilities);

    std::uint32_t imageCount = details.capabilities.minImageCount + 1;
    if(details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo {};
    createInfo.surface = this->surface.get();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    auto [graphicsQueueFamily, graphicsQueueIndex] = findGraphicsQueueFamility(this->physicalDevice).value();
    auto [presentQueueFamily, presentQueueIndex] = findPresentQueueFamility(this->physicalDevice).value();
    std::uint32_t queueFamilyIndices[] = {
        graphicsQueueIndex,
        presentQueueIndex
    };

    if(graphicsQueueFamily != presentQueueFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    auto swapchain = this->device->createSwapchainKHRUnique(createInfo);
    this->swapchainImages = this->device->getSwapchainImagesKHR(swapchain.get());
    this->swapchainImageFormat = surfaceFormat.format;
    this->swapchainExtent = extent;

    return swapchain;
}

std::vector<vk::UniqueImageView> VulkanEngine::createImageViews() {
    std::vector<vk::UniqueImageView> imageViews;
    for(auto& swapchainImage : swapchainImages) {
        vk::ImageViewCreateInfo createInfo {};
        createInfo.image = swapchainImage;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapchainImageFormat;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        imageViews.push_back(device->createImageViewUnique(createInfo));
    }
    return imageViews;
}

vk::UniqueDebugUtilsMessengerEXT VulkanEngine::createDebugMessenger() {
    assert(this->instance.get() != static_cast<vk::Instance>(nullptr));

    vk::DebugUtilsMessengerCreateInfoEXT createInfo {};
    createInfo.messageSeverity = 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    createInfo.messageType = 
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    return instance->createDebugUtilsMessengerEXTUnique(createInfo);
}

std::vector<vk::UniqueFramebuffer> VulkanEngine::createFramebuffers(vk::RenderPass renderPass) {
    assert(this->device.get() != static_cast<vk::Device>(nullptr) && !this->swapchainImageViews.empty());

    std::vector<vk::UniqueFramebuffer> framebuffers;
    for(auto& imageView : this->swapchainImageViews) {
        vk::ImageView attachments[] = {
            imageView.get()
        };

        vk::FramebufferCreateInfo framebufferInfo {};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;
        framebuffers.push_back(this->device->createFramebufferUnique(framebufferInfo));
    }
    return framebuffers;
}

vk::UniqueCommandPool VulkanEngine::createCommandPool() {
    assert(this->device.get() != static_cast<vk::Device>(nullptr));

    auto [ignore, index] = findGraphicsQueueFamility(this->physicalDevice).value();

    vk::CommandPoolCreateInfo poolInfo {};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = index;

    return this->device->createCommandPoolUnique(poolInfo);
}

vk::UniqueSemaphore VulkanEngine::createImageAvailableSemaphore() {
    assert(this->device.get() != static_cast<vk::Device>(nullptr));

    vk::SemaphoreCreateInfo semaphoreInfo {};
    return this->device->createSemaphoreUnique(semaphoreInfo);
}

vk::UniqueSemaphore VulkanEngine::createRenderFinishedSemaphore() {
    assert(this->device.get() != static_cast<vk::Device>(nullptr));

    vk::SemaphoreCreateInfo semaphoreInfo {};
    return this->device->createSemaphoreUnique(semaphoreInfo);
}

vk::UniqueFence VulkanEngine::createInFlightFence() {
    assert(this->device.get() != static_cast<vk::Device>(nullptr));

    vk::FenceCreateInfo fenceInfo {};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    return this->device->createFenceUnique(fenceInfo);
}


vk::PhysicalDevice VulkanEngine::choosePhysicalDevice() {
    assert(this->instance.get() != static_cast<vk::Instance>(nullptr));

    auto physicalDevices = this->instance->enumeratePhysicalDevices();

    for(auto physicalDevice : physicalDevices) {
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        if(isDeviceSuitable(physicalDevice)) {
            return physicalDevice;
        }
    }

    throw std::runtime_error("no suitable PhysicalDevice");
}

vk::SurfaceFormatKHR VulkanEngine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for(auto& availableFormat : availableFormats) {
        if(availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanEngine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentMode) {
    for(auto& availablePresentMode : availablePresentMode) {
        if(availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanEngine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    assert(this->window != nullptr);

    if(capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(this->window, &width, &height);

        vk::Extent2D actualExtent = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::optional<std::pair<vk::QueueFamilyProperties, std::uint32_t>> VulkanEngine::findGraphicsQueueFamility(vk::PhysicalDevice& physicalDevice) {
    std::uint32_t index = 0;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for(auto queueFamily : queueFamilies) {
        if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            return std::make_pair(queueFamily, index);
        }
        index++;
    }
    return std::nullopt;
}

std::optional<std::pair<vk::QueueFamilyProperties, std::uint32_t>> VulkanEngine::findPresentQueueFamility(vk::PhysicalDevice& physicalDevice) {
    assert(this->surface.get() != static_cast<vk::SurfaceKHR>(nullptr));

    std::uint32_t index = 0;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for(auto queueFamily : queueFamilies) {
        if(physicalDevice.getSurfaceSupportKHR(index, this->surface.get())) {
            return std::make_pair(queueFamily, index);
        }
        index++;
    }
    return std::nullopt;
}

bool VulkanEngine::checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    std::set<std::string> availableLayerSet;
    std::transform(availableLayers.begin(), availableLayers.end(), std::inserter(availableLayerSet, availableLayerSet.begin()), [](auto& layerProperies) { return layerProperies.layerName; });

    auto requiredLayers  = getRequiredValidationLayers();

    for(auto requiredLayer : requiredLayers) {
        if(availableLayerSet.find(requiredLayer) == availableLayerSet.end()) {
            return false;
        }
    }
    return true;
}

bool VulkanEngine::isDeviceSuitable(vk::PhysicalDevice& physicalDevice) {
    auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> strExtensions;
    std::transform(extensions.begin(), extensions.end(), std::inserter(strExtensions, strExtensions.begin()), [](auto& extension) { return extension.extensionName; });
    auto requiredExtensions = getRequiredDeviceExtensions();
    for(auto& requiredExtension : requiredExtensions) {
        if(strExtensions.find(requiredExtension) == strExtensions.end()) {
            return false;
        }
    }
    auto details = this->querySwapchainSupport(physicalDevice);
    if(details.formats.empty() | details.presentModes.empty()) {
        return false;
    }

    return findGraphicsQueueFamility(physicalDevice).has_value() && findPresentQueueFamility(physicalDevice).has_value();
}

SwapchainSupportDetails VulkanEngine::querySwapchainSupport(vk::PhysicalDevice& physicalDevice) {
    SwapchainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.get());
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface.get());
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface.get());

    return details;
}

std::vector<std::string> VulkanEngine::getRequiredValidationLayers() {
    return std::vector<std::string> {
        "VK_LAYER_KHRONOS_validation"
    };
}

std::vector<std::string> VulkanEngine::getRequiredExtensions() {
    std::vector<std::string> extensions;

    std::uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

std::vector<std::string> VulkanEngine::getRequiredDeviceExtensions() {
    return std::vector<std::string> {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

extern "C" {
    VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
}