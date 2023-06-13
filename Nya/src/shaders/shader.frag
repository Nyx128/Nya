#version 460

layout(location = 0) in vec2 frag_uv;

layout(binding = 1) uniform sampler2D texSampler;


layout(location = 0) out vec4 outColor;

void main() {
    float divisions = 1024.0;
    vec2 uv = frag_uv * divisions;
    float uvx_f = floor(uv.x);
    float uvy_f = floor(uv.y);
    vec2 pix_uv = vec2(uvx_f/divisions, uvy_f/divisions);

    vec4 texColor = texture(texSampler, pix_uv);
    

    if(texColor.w == 0.0){//transparency
        discard;
    }
    
    outColor = texColor;
}