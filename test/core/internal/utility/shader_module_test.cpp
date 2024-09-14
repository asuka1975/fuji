#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <stdexcept>

#include <vulkan/vulkan.hpp>

#include <fuji/core/internal/utility/shader_module.hpp>

using namespace fuji::core::utility;

namespace {
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
        std::ifstream fin{TEST_VERT_SPV_FILE, std::ios::binary};
        std::vector<char> code { std::istreambuf_iterator<char> { fin }, std::istreambuf_iterator<char> {} };
        EXPECT_NO_THROW(ShaderModule shaderModule(device.get(), code, vk::ShaderStageFlagBits::eVertex));
    }

    TEST_F(Utility_ShaderModuleTest, AbnormalCase_InvalidCodeThrowError) {
        std::vector<char> code { };
        EXPECT_THROW(ShaderModule shaderModule(device.get(), code, vk::ShaderStageFlagBits::eVertex), std::runtime_error);
    }
}