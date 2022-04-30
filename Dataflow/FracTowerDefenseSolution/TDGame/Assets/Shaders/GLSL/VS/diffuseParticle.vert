#version 430 core

#define MAX_DIRECTIONAL_LIGHT_COUNT 3

layout(location = 0) in mat4 a_modelMat;
layout(location = 4) in vec3 a_position;
layout(location = 5) in vec3 a_normal;
layout(location = 6) in vec4 a_color;

struct DirectionalLightDirData
{
	vec3 Direction;
};

uniform mat4 u_vpMat;
uniform mat4 u_modelMat;
uniform DirectionalLightDirData u_directionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];
uniform int u_directionalLightCount;


out VS_OUT
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	float IsEmissive;
	vec3 DirectionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];

} OUT;

void main()
{
	mat4 modelMat = a_modelMat;
	OUT.IsEmissive = a_modelMat[0][3];
	modelMat[0][3] = 0;
	vec4 m = modelMat * vec4(a_position, 1.0);

	OUT.Position = m.xyz;
	OUT.Normal = a_normal;
	OUT.Color = a_color;
	//OUT.IsEmissive = (isEmissive > 0) ? true: false;
	for(int i = 0; i < u_directionalLightCount; i++)
	{
		OUT.DirectionalLightDirections[i] = normalize(u_directionalLightDirections[i].Direction);
	}

	gl_Position = u_vpMat * m;
}