#include <fuji/core/internal/utility/shader_module.hpp>

#include <cstdint>

fuji::core::utility::ShaderModule::ShaderModule(const vk::Device& device, const std::vector<char>& code, vk::ShaderStageFlagBits shaderStage)
        : shaderStage(shaderStage) {
    std::vector<std::uint32_t> data(code.size() / 4 + (code.size() % 4 ? 1 : 0));
    std::memcpy(data.data(), code.data(), code.size());
    vk::ShaderModuleCreateInfo createInfo {
        {},
        data.size(),
        data.data()
    };
    this->shaderModule = device.createShaderModuleUnique(createInfo);
}


const vk::ShaderModule& fuji::core::utility::ShaderModule::getHandle() const {
    return this->shaderModule.get();
}

vk::ShaderStageFlagBits fuji::core::utility::ShaderModule::getShaderStage() const {
    return this->shaderStage;
}