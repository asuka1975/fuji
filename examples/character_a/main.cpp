#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <glm/fwd.hpp>
#include <iterator>
#include <stdexcept>
#include <iostream>

#include <vector>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan_engine.hpp>


std::uint32_t findMemoryType(std::uint32_t typeFilter, vk::PhysicalDeviceMemoryProperties memoryProperties, vk::MemoryPropertyFlags properties) {
    for(std::uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i))
                && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

struct Vertex {
    glm::vec2 pos;
    glm::vec2 texCoord;
};

class Texture {
public:
    Texture(VulkanEngine& engine, const std::vector<glm::u8vec4>& imageData, std::uint32_t width, std::uint32_t height) {
        vk::BufferCreateInfo imageBufferInfo {};
        imageBufferInfo.size = sizeof(glm::vec4) * width * height;
        imageBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
        imageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
        this->buffer = engine.getDevice()->createBufferUnique(imageBufferInfo);

        vk::MemoryRequirements bufferMemoryRequirements = engine.getDevice()->getBufferMemoryRequirements(this->buffer.get());
        vk::PhysicalDeviceMemoryProperties imageMemoryProperties = engine.getPhysicalDevice().getMemoryProperties();
        vk::MemoryPropertyFlags imageProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        std::uint32_t imageMemoryType = findMemoryType(bufferMemoryRequirements.memoryTypeBits, imageMemoryProperties, imageProperties);
        vk::MemoryAllocateInfo imageMemoryAllocInfo {};
        imageMemoryAllocInfo.allocationSize = bufferMemoryRequirements.size;
        imageMemoryAllocInfo.memoryTypeIndex = imageMemoryType;
        this->deviceMemory = engine.getDevice()->allocateMemoryUnique(imageMemoryAllocInfo);
        void* imageMapData;
        engine.getDevice()->bindBufferMemory(this->buffer.get(), this->deviceMemory.get(), 0);
        if(engine.getDevice()->mapMemory(this->deviceMemory.get(), 0, imageBufferInfo.size, vk::MemoryMapFlags { 0 }, &imageMapData) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to map memory to vertex buffer");
        }
        std::memcpy(imageMapData, imageData.data(), imageBufferInfo.size);
        engine.getDevice()->unmapMemory(this->deviceMemory.get());

        vk::ImageCreateInfo imageInfo {};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = vk::Format::eR8G8B8A8Srgb;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.flags = vk::ImageCreateFlags { 0 };
        this->image = engine.getDevice()->createImageUnique(imageInfo);

        vk::MemoryRequirements imageMemoryRequirements = engine.getDevice()->getImageMemoryRequirements(this->image.get());
        vk::MemoryAllocateInfo imageAllocInfo {};
        imageAllocInfo.allocationSize = imageMemoryRequirements.size;
        imageAllocInfo.memoryTypeIndex = findMemoryType(imageMemoryRequirements.memoryTypeBits, imageMemoryProperties, vk::MemoryPropertyFlagBits::eDeviceLocal);
        this->imageMemory = engine.getDevice()->allocateMemoryUnique(imageAllocInfo);

        engine.getDevice()->bindImageMemory(this->image.get(), this->imageMemory.get(), 0);

        this->transitionImageLayout(engine, this->image.get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        this->copyBufferToImage(engine, this->buffer.get(), this->image.get(), width, height);
        this->transitionImageLayout(engine, this->image.get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        vk::ImageViewCreateInfo viewInfo {};
        viewInfo.image = this->image.get();
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = vk::Format::eR8G8B8A8Srgb;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        this->imageView = engine.getDevice()->createImageViewUnique(viewInfo);

        vk::SamplerCreateInfo samplerInfo {};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        this->sampler = engine.getDevice()->createSamplerUnique(samplerInfo);
    }
    ~Texture() = default;

    vk::Image& getImage() {
        return image.get();
    }

    vk::ImageView& getImageView() {
        return imageView.get();
    }

    vk::Sampler& getSampler() {
        return sampler.get();
    }
private:
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory deviceMemory;
    vk::UniqueDeviceMemory imageMemory;
    vk::UniqueImage image;
    vk::UniqueImageView imageView;
    vk::UniqueSampler sampler;
private:
    std::vector<vk::UniqueCommandBuffer> beginSingleTimeCommands(VulkanEngine& engine) {
        vk::CommandBufferAllocateInfo allocInfo {};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = engine.getCommandPool().get();
        allocInfo.commandBufferCount = 1;

        std::vector<vk::UniqueCommandBuffer> commandBuffers = engine.getDevice()->allocateCommandBuffersUnique(allocInfo);

        vk::CommandBufferBeginInfo beginInfo {};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffers[0]->begin(beginInfo);

        return commandBuffers;
    }

    void endSingleTimeCommands(VulkanEngine& engine, std::vector<vk::UniqueCommandBuffer> commandBuffers) {
        commandBuffers[0]->end();

        vk::SubmitInfo submitInfo {};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0].get();

        engine.getGraphicsQueue().submit({ submitInfo }, VK_NULL_HANDLE);
        engine.getGraphicsQueue().waitIdle();
    }

    void transitionImageLayout(VulkanEngine& engine, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        std::vector<vk::UniqueCommandBuffer> commandBuffers = this->beginSingleTimeCommands(engine);

        vk::ImageMemoryBarrier barrier {};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = this->image.get();
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;
        if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlags { 0 };
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else {
            throw std::runtime_error("unsupported layout transition");
        }

        commandBuffers[0]->pipelineBarrier(
            sourceStage, destinationStage,
            vk::DependencyFlags { 0 },
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(engine, std::move(commandBuffers));
    }

    void copyBufferToImage(VulkanEngine& engine, vk::Buffer buffer, vk::Image image, std::uint32_t width, std::uint32_t height) {
        auto commandBuffers = beginSingleTimeCommands(engine);

        vk::BufferImageCopy region {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = vk::Offset3D { 0, 0, 0 };
        region.imageExtent = vk::Extent3D {
            width,
            height,
            1
        };

        commandBuffers[0]->copyBufferToImage(
            buffer, 
            image,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &region);

        endSingleTimeCommands(engine, std::move(commandBuffers));
    }

};

int main() {
    if(glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(500, 500, "character_a", nullptr, nullptr);
    if(window == nullptr) {
        throw std::runtime_error("failed to create GLFWwindow");
    }

    {
        VulkanEngine engine { "character_a" };
        engine.initialize(window);

        if(!engine.isInitialized()) {
            throw std::runtime_error("failed to initialize Vulkan");
        }

        std::vector<glm::u8vec4> imageData(16 * 16);
        for(int i = 0; i < 16; i++) {
            for(int j = 0; j < 16; j++) {
                if(i % 2) {
                    imageData[i * 16 + j] = j % 2 ? glm::u8vec4(255) : glm::u8vec4(glm::u8vec3(0), 255);
                } else {
                    imageData[i * 16 + j] = j % 2 ? glm::u8vec4(glm::u8vec3(0), 255) : glm::u8vec4(255);
                }
            }
        }

        Texture texture { engine, imageData, 16, 16 };

        std::ifstream vert_fin { TEXTURE_VERT_SPV_FILE, std::ios::in | std::ios::binary };
        std::vector<char> vert_code { std::istreambuf_iterator<char>{ vert_fin }, std::istreambuf_iterator<char> {} };
        std::ifstream frag_fin { TEXTURE_FRAG_SPV_FILE, std::ios::in | std::ios::binary };
        std::vector<char> frag_code { std::istreambuf_iterator<char>{ frag_fin }, std::istreambuf_iterator<char> {} };

        auto vertShaderModule = engine.createShaderModule(vert_code);
        auto fragShaderModule = engine.createShaderModule(frag_code);

        vk::PipelineShaderStageCreateInfo vertShaderStageInfo {};
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = vertShaderModule.get();
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo {};
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = fragShaderModule.get();
        fragShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo shaderStages[] = {
            vertShaderStageInfo,
            fragShaderStageInfo
        };

        std::vector<Vertex> vertices = {
            { glm::vec2(-0.5, 0.5), glm::vec2(0, 1) },
            { glm::vec2(-0.5, -0.5), glm::vec2(0, 0) },
            { glm::vec2(0.5, 0.5), glm::vec2(1, 1) },
            { glm::vec2(0.5, -0.5), glm::vec2(1, 0) }
        };
        vk::BufferCreateInfo positionBufferInfo {};
        positionBufferInfo.size = sizeof(Vertex) * vertices.size();
        positionBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
        positionBufferInfo.sharingMode = vk::SharingMode::eExclusive;
        vk::UniqueBuffer positionBuffer = engine.getDevice()->createBufferUnique(positionBufferInfo);

        vk::MemoryRequirements memoryRequirements = engine.getDevice()->getBufferMemoryRequirements(positionBuffer.get());
        vk::PhysicalDeviceMemoryProperties memoryProperties = engine.getPhysicalDevice().getMemoryProperties();
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        std::uint32_t memoryType = findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties, properties);
        vk::MemoryAllocateInfo memoryAllocInfo {};
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = memoryType;
        vk::UniqueDeviceMemory positionBufferMemory = engine.getDevice()->allocateMemoryUnique(memoryAllocInfo);
        engine.getDevice()->bindBufferMemory(positionBuffer.get(), positionBufferMemory.get(), 0);
        void* mapData;
        if(engine.getDevice()->mapMemory(positionBufferMemory.get(), 0, positionBufferInfo.size, vk::MemoryMapFlags { 0 }, &mapData) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to map memory to vertex buffer");
        }
        std::memcpy(mapData, vertices.data(), positionBufferInfo.size);
        engine.getDevice()->unmapMemory(positionBufferMemory.get());

        std::array<vk::DescriptorPoolSize, 1> poolSizes {};
        poolSizes[0].type = vk::DescriptorType::eCombinedImageSampler;
        poolSizes[0].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo {};
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 2;
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        auto descriptorPool = engine.getDevice()->createDescriptorPoolUnique(poolInfo);

        vk::DescriptorSetLayoutBinding samplerLayoutBinding {};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {
            samplerLayoutBinding
        };
        vk::DescriptorSetLayoutCreateInfo layoutInfo {};
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();
        auto descriptorSetLayout = engine.getDevice()->createDescriptorSetLayoutUnique(layoutInfo);


        vk::DescriptorImageInfo imageInfo {};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = texture.getImageView();
        imageInfo.sampler = texture.getSampler();

        vk::DescriptorSetAllocateInfo descriptorSetAllocInfo {};
        descriptorSetAllocInfo.descriptorPool = descriptorPool.get();
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout.get();

        auto descriptorSets = engine.getDevice()->allocateDescriptorSetsUnique(descriptorSetAllocInfo);
        std::array<vk::WriteDescriptorSet, 1> descriptorWrites {};
        descriptorWrites[0].dstSet = descriptorSets[0].get();
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pImageInfo = &imageInfo;
        engine.getDevice()->updateDescriptorSets(descriptorWrites, {});
        
        vk::VertexInputBindingDescription positionBinding {};
        positionBinding.binding = 0;
        positionBinding.stride = sizeof(Vertex);
        positionBinding.inputRate = vk::VertexInputRate::eVertex;

        vk::VertexInputAttributeDescription positionAttribute {};
        positionAttribute.binding = 0;
        positionAttribute.location = 0;
        positionAttribute.format = vk::Format::eR32G32Sfloat;
        positionAttribute.offset = offsetof(Vertex, pos);

        vk::VertexInputAttributeDescription textureCoordAttribute {};
        textureCoordAttribute.binding = 0;
        textureCoordAttribute.location = 1;
        textureCoordAttribute.format = vk::Format::eR32G32Sfloat;
        textureCoordAttribute.offset = offsetof(Vertex, texCoord);

        std::vector<vk::VertexInputAttributeDescription> attributes = {
            positionAttribute, textureCoordAttribute
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &positionBinding;
        vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
        vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly {};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleStrip;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::Viewport viewport {
            0.0f, 0.0f,
            static_cast<float>(engine.getSwapchainExtent().width), 
            static_cast<float>(engine.getSwapchainExtent().height),
            0.0f, 1.0f
        };

        vk::Rect2D scissor {
            { 0, 0 },
            engine.getSwapchainExtent()
        };

        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamicState {};
        dynamicState.dynamicStateCount = dynamicStates.size();
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineViewportStateCreateInfo viewportState {};
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        vk::PipelineMultisampleStateCreateInfo multisampling {};
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask = 
            vk::ColorComponentFlagBits::eR
          | vk::ColorComponentFlagBits::eG
          | vk::ColorComponentFlagBits::eB
          | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.get();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        vk::UniquePipelineLayout pipelineLayout = engine.getDevice()->createPipelineLayoutUnique(pipelineLayoutInfo);

        vk::AttachmentDescription colorAttachment {};
        colorAttachment.format = engine.getFormat();
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlags { 0 };
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo renderPassInfo {};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::UniqueRenderPass renderPass = engine.getDevice()->createRenderPassUnique(renderPassInfo);
        vk::GraphicsPipelineCreateInfo graphicsPipelineInfo {};
        graphicsPipelineInfo.stageCount = 2;
        graphicsPipelineInfo.pStages = shaderStages;
        graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
        graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
        graphicsPipelineInfo.pViewportState = &viewportState;
        graphicsPipelineInfo.pRasterizationState = &rasterizer;
        graphicsPipelineInfo.pMultisampleState = &multisampling;
        graphicsPipelineInfo.pDepthStencilState = nullptr;
        graphicsPipelineInfo.pColorBlendState = &colorBlending;
        graphicsPipelineInfo.pDynamicState = &dynamicState;
        graphicsPipelineInfo.layout = pipelineLayout.get();
        graphicsPipelineInfo.renderPass = renderPass.get();
        graphicsPipelineInfo.subpass = 0;
        graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        std::vector<vk::GraphicsPipelineCreateInfo> graphicsPipelineInfos = {
            graphicsPipelineInfo
        };

        auto [result, graphicsPipeline] = engine.getDevice()->createGraphicsPipelinesUnique(VK_NULL_HANDLE, graphicsPipelineInfos);
        if(result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create GraphicsPipeline");
        }

        vk::CommandBufferAllocateInfo allocInfo {};
        allocInfo.commandPool = engine.getCommandPool().get();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        auto commandBuffers = engine.getDevice()->allocateCommandBuffersUnique(allocInfo);


        std::vector<vk::DescriptorSet> copiedDescriptorSets = {
            descriptorSets[0].get()
        };

        engine.setCommandBufferRecorder([
            &commandBuffer = commandBuffers[0].get(),
            &renderPass = renderPass.get(),
            &graphicsPipeline = graphicsPipeline[0].get(),
            extent = engine.getSwapchainExtent(),
            &positionBuffer = positionBuffer.get(),
            &descriptorSets = copiedDescriptorSets,
            &pipelineLayout = pipelineLayout
        ](vk::Framebuffer framebuffer) {
            vk::CommandBufferBeginInfo beginInfo {};
            commandBuffer.begin(beginInfo);

            vk::RenderPassBeginInfo renderPassInfo {};
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffer;
            renderPassInfo.renderArea.offset = vk::Offset2D { 0, 0 };
            renderPassInfo.renderArea.extent = extent;

            vk::ClearValue clearColor = { 
                vk::ClearColorValue { std::array<float, 4> { 0.0f, 0.0f, 0.0f, 1.0f } }
            };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

                vk::Viewport viewport {
                    0.0f, 0.0f,
                    static_cast<float>(extent.width),
                    static_cast<float>(extent.height),
                    0.0f, 1.0f
                };
                commandBuffer.setViewport(0, { viewport });

                vk::Rect2D scissor {
                    vk::Offset2D { 0, 0 },
                    extent
                };
                commandBuffer.setScissor(0, { scissor });

                commandBuffer.bindVertexBuffers(0, { positionBuffer }, { 0 });

                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout.get(), 0, descriptorSets, {});

                commandBuffer.draw(4, 1, 0, 0);

            commandBuffer.endRenderPass();

            commandBuffer.end();
        });

        engine.setupRenderTarget(renderPass.get());

        while(glfwWindowShouldClose(window) == GLFW_FALSE) {
            glfwPollEvents();
            engine.drawFrame(commandBuffers[0].get());
        }

        engine.getDevice()->waitIdle();
    }

    glfwTerminate();
}