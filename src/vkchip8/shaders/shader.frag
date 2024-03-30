#version 450

layout(binding = 1) uniform UniformBufferObject {
    vec4 fragColor;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = ubo.fragColor;
}
