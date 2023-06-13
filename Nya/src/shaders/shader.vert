#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec2 frag_uv;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConsts {
    mat4 transform;
} pushConsts;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model* vec4(pos, 1.0) ;
    frag_uv = uv;
}