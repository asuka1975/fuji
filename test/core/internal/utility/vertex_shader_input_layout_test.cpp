#include <cstdint>
#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <stdexcept>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <fuji/core/internal/utility/vertex_shader_input_layout.hpp>

using namespace fuji::core::utility;

namespace {
    struct Vertex1 {
        glm::vec2 attribute1;
        glm::vec4 attribute2;
    };

    struct Vertex2 {
        std::uint8_t attribute1;
        glm::mat4 attribute2;
    };

    TEST(Utility_VertexShaderInputLayoutTest, NormalCase_Binding1) {
        Binding binding(&Vertex1::attribute1, &Vertex1::attribute2);

        EXPECT_EQ(sizeof(glm::vec2) + sizeof(glm::vec4), binding.getStride());
        EXPECT_EQ(0, binding.getAttributeDescriptions()[0].offset);
        EXPECT_EQ(vk::Format::eR32G32Sfloat, binding.getAttributeDescriptions()[0].format);
        EXPECT_EQ(8, binding.getAttributeDescriptions()[1].offset);
        EXPECT_EQ(vk::Format::eR32G32B32A32Sfloat, binding.getAttributeDescriptions()[1].format);
    }

    TEST(Utility_VertexShaderInputLayoutTest, NormalCase_Binding2) {
        Binding binding(&Vertex2::attribute1, &Vertex2::attribute2);

        EXPECT_EQ(4 + sizeof(glm::mat4), binding.getStride());
        EXPECT_EQ(0, binding.getAttributeDescriptions()[0].offset);
        EXPECT_EQ(vk::Format::eR8Uint, binding.getAttributeDescriptions()[0].format);
        EXPECT_EQ(4, binding.getAttributeDescriptions()[1].offset);
        EXPECT_EQ(vk::Format::eAstc4x4SfloatBlock, binding.getAttributeDescriptions()[1].format);
    }

    TEST(Utility_VertexShaderInputLayoutTest, NormalCase_InstantiateInputLayout) {
        VertexShaderInputLayout inputLayout({
            Binding { &Vertex1::attribute1, &Vertex1::attribute2 },
            Binding { &Vertex2::attribute1, &Vertex2::attribute2 },
        });

        EXPECT_EQ(sizeof(glm::vec2) + sizeof(glm::vec4), inputLayout.getBindings()[0].getStride());
        EXPECT_EQ(0, inputLayout.getBindings()[0].getAttributeDescriptions()[0].offset);
        EXPECT_EQ(vk::Format::eR32G32Sfloat, inputLayout.getBindings()[0].getAttributeDescriptions()[0].format);
        EXPECT_EQ(8, inputLayout.getBindings()[0].getAttributeDescriptions()[1].offset);
        EXPECT_EQ(vk::Format::eR32G32B32A32Sfloat, inputLayout.getBindings()[0].getAttributeDescriptions()[1].format);
        EXPECT_EQ(4 + sizeof(glm::mat4), inputLayout.getBindings()[1].getStride());
        EXPECT_EQ(0, inputLayout.getBindings()[1].getAttributeDescriptions()[0].offset);
        EXPECT_EQ(vk::Format::eR8Uint, inputLayout.getBindings()[1].getAttributeDescriptions()[0].format);
        EXPECT_EQ(4, inputLayout.getBindings()[1].getAttributeDescriptions()[1].offset);
        EXPECT_EQ(vk::Format::eAstc4x4SfloatBlock, inputLayout.getBindings()[1].getAttributeDescriptions()[1].format);
    }
}