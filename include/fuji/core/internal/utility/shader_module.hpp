#ifndef INCLUDE_FUJI_CORE_UTILITY_SHADER_MODULE_HPP
#define INCLUDE_FUJI_CORE_UTILITY_SHADER_MODULE_HPP

#include <vulkan/vulkan.hpp>

namespace fuji::core::utility {
    class ShaderModule {
    public:
        ShaderModule(const vk::Device& device, const std::vector<char>& code, vk::ShaderStageFlagBits shaderStage);
        ~ShaderModule() = default;
        const vk::ShaderModule& getHandle() const;
        vk::ShaderStageFlagBits getShaderStage() const;
    private:
        vk::UniqueShaderModule shaderModule;
        vk::ShaderStageFlagBits shaderStage;
    };
}

#endif