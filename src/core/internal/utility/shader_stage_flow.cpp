#include <algorithm>
#include <fuji/core/internal/utility/shader_stage_flow.hpp>
#include <iterator>


fuji::core::utility::ShaderStageFlow::ShaderStageFlow(std::vector<ShaderModule> shaderModules, VertexShaderInputLayout inputLayout)
        : shaderModules(std::make_move_iterator(shaderModules.begin()), std::make_move_iterator(shaderModules.end())),
          inputLayout(std::move(inputLayout)) {
    std::transform(this->shaderModules.begin(), this->shaderModules.end(), std::back_inserter(this->shaderStageCreateInfos), [](ShaderModule& module) {
        return vk::PipelineShaderStageCreateInfo {
            {},
            module.getShaderStage(),
            module.getHandle(),
            "main"
        };
    });
    for(std::uint32_t binding = 0, location = 0; binding < this->inputLayout.getBindings().size(); binding++) {
        auto& bindingDescription = this->inputLayout.getBindings()[binding];
        this->bindingDescriptions.emplace_back(
            binding, bindingDescription.getStride(), bindingDescription.getInputRate()
        );

        for(auto& attributeDescription : bindingDescription.getAttributeDescriptions()) {
            this->attributeDescriptions.emplace_back(
                location, binding, attributeDescription.format, attributeDescription.offset
            );
            location++;
        }
    }
    this->vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo {
        {}, 
        this->bindingDescriptions,
        this->attributeDescriptions
    };
}

const vk::PipelineVertexInputStateCreateInfo& fuji::core::utility::ShaderStageFlow::getVertexInputStateCreateInfo() const noexcept {
    return this->vertexInputStateCreateInfo;
}

const std::vector<vk::PipelineShaderStageCreateInfo>& fuji::core::utility::ShaderStageFlow::getShaderStageCreateInfos() const noexcept {
    return this->shaderStageCreateInfos;
}