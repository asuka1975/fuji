#version 460

layout (location = 0) in vec2 position; // binding #0
layout (location = 1) in vec2 coord;    // binding #0
layout (location = 2) in mat4 instance; // binding #1

layout (location = 0) out vec2 outCoord;

void main() {
    gl_Position = vec4(position, 0, 1) * instance;
    outCoord = coord;
}