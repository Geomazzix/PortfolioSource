#version 430 core

#define MAX_POINT_LIGHT_COUNT 18
#define MAX_DIRECTIONAL_LIGHT_COUNT 3

const vec3 fogPlaneNormal = vec3(0,1,0);

layout(location = 0) in mat4 a_modelMat;
layout(location = 4) in vec3 a_position;
layout(location = 5) in vec3 a_normal;
layout(location = 6) in vec4 a_tangent;
layout(location = 7) in vec2 a_texCoord;
layout(location = 8) in vec4 a_color;
layout(location = 9) in mat3 a_normalMat;

uniform mat4 u_vpMat;
uniform vec3 u_cameraPos;

struct PointLightVertexData
{
	vec3 Position;
};

struct DirectionalLightDirData
{
	vec3 Direction;
};

uniform PointLightVertexData u_pointLightPositions[MAX_POINT_LIGHT_COUNT];
uniform int u_pointLightCount;
uniform DirectionalLightDirData u_directionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];
uniform int u_directionalLightCount;
uniform mat4 u_lightSpaceMatrix;

out VS_OUT
{
	vec2 TexCoords;
	vec4 VertexColor;
	vec3 Position;
	vec3 TS_Position;
	vec3 TS_ViewDir;
	vec3 TS_DirectionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];
	vec3 TS_PointLightPositions[MAX_POINT_LIGHT_COUNT];		//TS = tangent space.
	vec4 LS_Position;										//LS = lightspace.

	float FogMagicValue1;
    float FogMagicValue2;
} OUT;

void main()
{
	vec4 p = a_modelMat * vec4(a_position, 1.0);
	OUT.Position = p.xyz;
	OUT.TexCoords = a_texCoord;
	OUT.VertexColor = a_color;

    vec3 t = normalize(a_normalMat * a_tangent.xyz);
    vec3 n = normalize(a_normalMat * a_normal);
    t = normalize(t - dot(t, n) * n);
	vec3 b = cross(n, t) * a_tangent.w;

	mat3 tbnMat = transpose(mat3(t, b, n));
	OUT.TS_Position = tbnMat * p.xyz;
	OUT.TS_ViewDir = normalize(tbnMat * (u_cameraPos - p.xyz));

	for(int i = 0; i < u_directionalLightCount; i++)
	{
		OUT.TS_DirectionalLightDirections[i] = normalize(tbnMat * u_directionalLightDirections[i].Direction);
	}

	OUT.LS_Position = u_lightSpaceMatrix * p;

	for(int i = 0; i < u_pointLightCount; i++)
	{
		OUT.TS_PointLightPositions[i] = tbnMat * u_pointLightPositions[i].Position;
 	}

	int constantM = 0;
	
	if (dot(fogPlaneNormal,u_cameraPos) < 0) constantM = 1;
	if (p.y > 5) constantM = 0;
	// Fog variables:
	OUT.FogMagicValue1 = constantM * (dot(fogPlaneNormal,u_cameraPos) + dot(fogPlaneNormal,vec3(p.xyz)));
	OUT.FogMagicValue2 = dot(fogPlaneNormal,vec3(p.xyz)) * sign(dot(fogPlaneNormal,u_cameraPos));

	gl_Position = u_vpMat * p;
}