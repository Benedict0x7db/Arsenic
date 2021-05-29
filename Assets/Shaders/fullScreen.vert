#version 450

#include "structures.glsl"

layout(location = 0) out vec2 outUV;

vec4 verticles[] = vec4[](
    vec4(-1.0f, -1.0f, 0.0f, 1.0f),
    vec4(1.0f, -1.0f, 1.0f, 1.0f),
    vec4(1.0f, 1.0f, 1.0f, 0.0f),
    vec4(1.0f, 1.0f, 1.0f, 0.0f),
    vec4(-1.0f, 1.0f, 0.0f, 0.0f),
    vec4(-1.0f, -1.0f, 0.0f, 1.0f)
);

void main()
{
    vec4 vertex = verticles[gl_VertexIndex];
    outUV = vec2(vertex.zw);
    
    gl_Position = vec4(vertex.xy, 0, 1);
}