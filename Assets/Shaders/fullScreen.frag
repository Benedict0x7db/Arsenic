#version 450

#include "structures.glsl"

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 color = texture(rtRenderTarget, inUV).rgb;
    fragColor = vec4(color, 1.0f);
}