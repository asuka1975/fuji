#ifndef INCLUDE_FUJI_CORE_INSTANCE_HPP
#define INCLUDE_FUJI_CORE_INSTANCE_HPP

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace fuji {
    class Instance {
    public:
        Instance(vk::UniqueDevice& device);
        ~Instance() = default;
        void draw(vk::UniqueFramebuffer& framebuffer);
    private:
        vk::UniqueRenderPass renderPass;
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout pipelineLayout;
    };
}

#endif
