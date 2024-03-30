#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inOffset;

layout(binding = 0) uniform UniformBufferObject {
    vec2 pixelScale;
} ubo;

void main() {
    gl_Position = vec4(inPosition * ubo.pixelScale + inOffset, 0.0, 1.0);
}
