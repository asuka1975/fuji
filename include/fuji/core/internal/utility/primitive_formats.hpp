#ifndef INCLUDE_FUJI_CORE_UTILITY_PRIMITIVE_FORMATS_HPP
#define INCLUDE_FUJI_CORE_UTILITY_PRIMITIVE_FORMATS_HPP

#include <glm/fwd.hpp>
#include <vulkan/vulkan.hpp>


#define  DEFINE_PRIMITIVE_FORMAT(target_type, target_value)   \
template <> \
struct PrimitiveFormats<target_type> { \
    using type = target_type; \
    inline static constexpr vk::Format value = target_value; \
};


namespace fuji::core::utility {
    template <class T>
    struct NoFormat {};

    template <class T>
    struct PrimitiveFormats {
        using type = void;
        inline static constexpr vk::Format value = NoFormat<T>::value;
    };

    DEFINE_PRIMITIVE_FORMAT(float, vk::Format::eR32Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::vec2, vk::Format::eR32G32Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::vec3, vk::Format::eR32G32B32Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::vec4, vk::Format::eR32G32B32A32Sfloat);
    DEFINE_PRIMITIVE_FORMAT(double, vk::Format::eR64Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::dvec2, vk::Format::eR64G64Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::dvec3, vk::Format::eR64G64B64Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::dvec4, vk::Format::eR64G64B64A64Sfloat);
    DEFINE_PRIMITIVE_FORMAT(glm::mat4x4, vk::Format::eAstc4x4SfloatBlock);

    DEFINE_PRIMITIVE_FORMAT(std::int8_t, vk::Format::eR8Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i8vec2, vk::Format::eR8G8Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i8vec3, vk::Format::eR8G8B8Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i8vec4, vk::Format::eR8G8B8A8Sint);
    DEFINE_PRIMITIVE_FORMAT(std::int16_t, vk::Format::eR16Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i16vec2, vk::Format::eR16G16Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i16vec3, vk::Format::eR16G16B16Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i16vec4, vk::Format::eR16G16B16A16Sint);
    DEFINE_PRIMITIVE_FORMAT(std::int32_t, vk::Format::eR32Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i32vec2, vk::Format::eR32G32Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i32vec3, vk::Format::eR32G32B32Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i32vec4, vk::Format::eR32G32B32A32Sint);
    DEFINE_PRIMITIVE_FORMAT(std::int64_t, vk::Format::eR64Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i64vec2, vk::Format::eR64G64Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i64vec3, vk::Format::eR64G64B64Sint);
    DEFINE_PRIMITIVE_FORMAT(glm::i64vec4, vk::Format::eR64G64B64A64Sint);
    DEFINE_PRIMITIVE_FORMAT(std::uint8_t, vk::Format::eR8Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u8vec2, vk::Format::eR8G8Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u8vec3, vk::Format::eR8G8B8Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u8vec4, vk::Format::eR8G8B8A8Uint);
    DEFINE_PRIMITIVE_FORMAT(std::uint16_t, vk::Format::eR16Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u16vec2, vk::Format::eR16G16Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u16vec3, vk::Format::eR16G16B16Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u16vec4, vk::Format::eR16G16B16A16Uint);
    DEFINE_PRIMITIVE_FORMAT(std::uint32_t, vk::Format::eR32Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u32vec2, vk::Format::eR32G32Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u32vec3, vk::Format::eR32G32B32Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u32vec4, vk::Format::eR32G32B32A32Uint);
    DEFINE_PRIMITIVE_FORMAT(std::uint64_t, vk::Format::eR64Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u64vec2, vk::Format::eR64G64Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u64vec3, vk::Format::eR64G64B64Uint);
    DEFINE_PRIMITIVE_FORMAT(glm::u64vec4, vk::Format::eR64G64B64A64Uint);
}

#endif