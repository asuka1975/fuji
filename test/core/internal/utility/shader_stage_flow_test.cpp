#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>

#include <fstream>
#include <initializer_list>
#include <iterator>
#include <stdexcept>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <fuji/core/internal/utility/shader_stage_flow.hpp>

namespace {
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 coord;
    };

    struct Instance {
        glm::mat4 model;
    };

    class Utility_ShaderModuleTest : public testing::Test {
    protected:
        void SetUp() override {
            this->instance = vk::createInstanceUnique({});
            vk::PhysicalDevice physicalDevice = this->instance->enumeratePhysicalDevices()[0];
            this->device = physicalDevice.createDeviceUnique({});
        }
        void TearDown() override {
            device.reset();
            instance.reset();
        }
    protected:
        vk::UniqueInstance instance;
        vk::UniqueDevice device;
    };

    TEST_F(Utility_ShaderModuleTest, NormalCase_Nothrow) {
        std::ifstream vert_fin { TEST_VERT_SPV_FILE, std::ios::in | std::ios::binary };
        std::vector<char> vert_code { std::istreambuf_iterator<char>{ vert_fin }, std::istreambuf_iterator<char> {} };
        std::ifstream frag_fin { TEST_FRAG_SPV_FILE, std::ios::in | std::ios::binary };
        std::vector<char> frag_code { std::istreambuf_iterator<char>{ frag_fin }, std::istreambuf_iterator<char> {} };

        std::vector<fuji::core::utility::ShaderModule> shaderModules;
        shaderModules.emplace_back(device.get(), vert_code, vk::ShaderStageFlagBits::eVertex);
        shaderModules.emplace_back(device.get(), frag_code, vk::ShaderStageFlagBits::eFragment);

        fuji::core::utility::ShaderStageFlow shaderStageFlow {
            std::move(shaderModules),
            fuji::core::utility::VertexShaderInputLayout {
                {
                    fuji::core::utility::Binding { vk::VertexInputRate::eVertex, &Vertex::pos, &Vertex::coord },
                    fuji::core::utility::Binding { vk::VertexInputRate::eInstance, &Instance::model },
                }
            }
        };

        EXPECT_EQ(vk::ShaderStageFlagBits::eVertex, shaderStageFlow.getShaderStageCreateInfos()[0].stage);
        EXPECT_STREQ("main", shaderStageFlow.getShaderStageCreateInfos()[0].pName);
        EXPECT_EQ(vk::ShaderStageFlagBits::eFragment, shaderStageFlow.getShaderStageCreateInfos()[1].stage);
        EXPECT_STREQ("main", shaderStageFlow.getShaderStageCreateInfos()[1].pName);

        EXPECT_EQ(3, shaderStageFlow.getVertexInputStateCreateInfo().vertexAttributeDescriptionCount);
        EXPECT_EQ(2, shaderStageFlow.getVertexInputStateCreateInfo().vertexBindingDescriptionCount);
    }
}