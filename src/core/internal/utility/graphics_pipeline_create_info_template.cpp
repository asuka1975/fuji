#include "fuji/core/internal/utility/shader_stage_flow.hpp"
#include "fuji/core/internal/utility/vertex_shader_input_layout.hpp"
#include <algorithm>
#include <fuji/core/internal/utility/graphics_pipeline_create_info_template.hpp>

#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

fuji::core::utility::GraphicsPipelineCreateInfoTemplate::GraphicsPipelineCreateInfoTemplate(std::unique_ptr<ShaderStageFlow> shaderStageFlow) 
        : shaderStageFlow(std::move(shaderStageFlow)) {
    
}

std::vector<vk::DynamicState> fuji::core::utility::GraphicsPipelineCreateInfoTemplate::getDynamicStates() const noexcept { 
    return std::vector<vk::DynamicState> {};
}

vk::GraphicsPipelineCreateInfo fuji::core::utility::GraphicsPipelineCreateInfoTemplate::getCreateInfo() const noexcept {
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
        {}, 
        this->shaderStageFlow->getShaderStageCreateInfos(),
        &this->shaderStageFlow->getVertexInputStateCreateInfo()
    };

    return graphicsPipelineCreateInfo;
}

std::vector<vk::UniquePipeline> fuji::core::utility::GraphicsPipelineCreateInfoTemplate::createGraphicsPipeline(vk::Device &device, vk::ArrayProxy<std::unique_ptr<GraphicsPipelineCreateInfoTemplate>> graphicsPipelineCreateInfos) {
    std::vector<vk::GraphicsPipelineCreateInfo> createInfos;
    std::transform(graphicsPipelineCreateInfos.begin(), graphicsPipelineCreateInfos.end(), std::back_inserter(createInfos), [](auto& info) { return info->getCreateInfo(); });

    auto [result, value] = device.createGraphicsPipelinesUnique(VK_NULL_HANDLE, createInfos);
    if(result != vk::Result::eSuccess) {
        vk::throwResultException(result, "failed to create GraphicsPipelines");
    }
    return std::move(value);
}