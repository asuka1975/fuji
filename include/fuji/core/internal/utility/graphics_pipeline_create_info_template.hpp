#ifndef INCLUDE_FUJI_CORE_UTILITY_GRAPHICS_PIPELINE_CREATE_INFO_TEMPLATE_HPP
#define INCLUDE_FUJI_CORE_UTILITY_GRAPHICS_PIPELINE_CREATE_INFO_TEMPLATE_HPP

#include <memory>

#include <vulkan/vulkan.hpp>

#include "fuji/core/internal/utility/shader_stage_flow.hpp"

namespace fuji::core::utility {
    class GraphicsPipelineCreateInfoTemplate {
    public:
        GraphicsPipelineCreateInfoTemplate(std::unique_ptr<ShaderStageFlow> shaderStageFlow);
        virtual ~GraphicsPipelineCreateInfoTemplate() = default;
        virtual std::vector<vk::DynamicState> getDynamicStates() const noexcept;
        vk::GraphicsPipelineCreateInfo getCreateInfo() const noexcept;

        static std::vector<vk::UniquePipeline> createGraphicsPipeline(vk::Device& device, vk::ArrayProxy<std::unique_ptr<GraphicsPipelineCreateInfoTemplate>> graphicsPipelineCreateInfos);
    private:
        std::unique_ptr<ShaderStageFlow> shaderStageFlow;
    };
}

#endif