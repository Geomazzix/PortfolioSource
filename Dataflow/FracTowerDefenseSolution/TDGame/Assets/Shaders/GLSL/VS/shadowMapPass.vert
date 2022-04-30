#version 430 core

layout(location = 0) in mat4 a_modelMat;
layout(location = 4) in vec3 a_position;
layout(location = 5) in vec3 a_normal;
layout(location = 6) in vec4 a_tangent;
layout(location = 7) in vec2 a_texCoord;


uniform mat4 u_lightSpaceMatrix;

void main()
{
    mat4 modelMat = a_modelMat;
    modelMat[0][3] = 0;
    gl_Position = u_lightSpaceMatrix * modelMat * vec4(a_position, 1.0);
}  