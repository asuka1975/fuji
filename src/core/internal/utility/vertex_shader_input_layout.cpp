#include <fuji/core/internal/utility/vertex_shader_input_layout.hpp>
#include <initializer_list>
#include <vector>

const std::vector<fuji::core::utility::AttributeDescription>& fuji::core::utility::Binding::getAttributeDescriptions() const noexcept {
    return this->attributeDescriptions;
}

std::size_t fuji::core::utility::Binding::getStride() const noexcept {
    return this->stride;
}


fuji::core::utility::VertexShaderInputLayout::VertexShaderInputLayout(std::initializer_list<Binding> bindings) : bindings(bindings.begin(), bindings.end()) {}

const std::vector<fuji::core::utility::Binding>& fuji::core::utility::VertexShaderInputLayout::getBindings() const noexcept { 
    return this->bindings;
}