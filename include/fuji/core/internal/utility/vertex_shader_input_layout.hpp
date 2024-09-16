#ifndef INCLUDE_FUJI_CORE_UTILITY_VERTEX_SHADER_INPUT_LAYOUT_HPP
#define INCLUDE_FUJI_CORE_UTILITY_VERTEX_SHADER_INPUT_LAYOUT_HPP

#include <cstddef>
#include <initializer_list>
#include <vulkan/vulkan.hpp>

#include <fuji/core/internal/utility/primitive_formats.hpp>

namespace fuji::core::utility {
    struct AttributeDescription {
        vk::Format format;
        std::uint32_t offset;

        template <class T, class U>
        static AttributeDescription createDescription(U T::* member) {
            return {
                PrimitiveFormats<U>::value,
                static_cast<std::uint32_t>(reinterpret_cast<std::size_t>(&((T*)nullptr->*member)))
            };
        }
    };

    class Binding {
    public:
        template <class T, class... U>
        Binding(vk::VertexInputRate inputRate, U T::*... members) : inputRate(inputRate), attributeDescriptions({ AttributeDescription::createDescription(members)... }), stride(sizeof(T)) {}
        vk::VertexInputRate getInputRate() const noexcept;
        const std::vector<AttributeDescription>& getAttributeDescriptions() const noexcept;
        std::uint32_t getStride() const noexcept;
    private:
        vk::VertexInputRate inputRate;
        std::vector<AttributeDescription> attributeDescriptions;
        std::uint32_t stride;
    };

    class VertexShaderInputLayout {
    public:
        VertexShaderInputLayout(std::initializer_list<Binding> bindings);
        const std::vector<Binding>& getBindings() const noexcept;
    private:
        std::vector<Binding> bindings;
    };
}

#endif