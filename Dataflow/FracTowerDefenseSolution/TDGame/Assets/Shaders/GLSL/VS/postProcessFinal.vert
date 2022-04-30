#version 430 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texCoords;

out VS_OUT
{
    vec2 TexCoords;
} OUT;

void main()
{
    OUT.TexCoords = a_texCoords;
    gl_Position = vec4(a_position, 1.0);
}