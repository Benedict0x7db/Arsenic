#version 450

#include "structures.glsl"

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(rtRenderTarget, inUV).rgb, 1.0f);
}