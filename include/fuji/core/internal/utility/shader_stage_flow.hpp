#ifndef INCLUDE_FUJI_CORE_UTILITY_SHADER_STAGE_FLOW_HPP
#define INCLUDE_FUJI_CORE_UTILITY_SHADER_STAGE_FLOW_HPP

#include <vulkan/vulkan.hpp>

#include <fuji/core/internal/utility/shader_module.hpp>
#include <fuji/core/internal/utility/vertex_shader_input_layout.hpp>

namespace fuji::core::utility {
    class ShaderStageFlow {
    public:
        ShaderStageFlow(std::vector<ShaderModule> shaderModules, VertexShaderInputLayout inputLayout);
        const vk::PipelineVertexInputStateCreateInfo& getVertexInputStateCreateInfo() const noexcept;
        const std::vector<vk::PipelineShaderStageCreateInfo>& getShaderStageCreateInfos() const noexcept;
    private:
        std::vector<ShaderModule> shaderModules;
        VertexShaderInputLayout inputLayout;
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
        vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    };
}

#endif