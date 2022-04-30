#version 430 core

#define MAX_POINT_LIGHT_COUNT 18
#define MAX_DIRECTIONAL_LIGHT_COUNT 3

struct DirectionalLight
{
    vec3 Color;
    vec3 Direction;
    float Intensity;
};

struct PointLight
{
    vec3 Position;
    vec3 Color;
    float Intensity;
    float Range;
};

uniform PointLight u_pointLights[MAX_POINT_LIGHT_COUNT];
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
uniform int u_directionalLightCount;

uniform bool u_hasDirectionalLight;
uniform int u_pointLightCount;
uniform vec3 u_cameraPos;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor;

in VS_OUT
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	float IsEmissive;
    vec3 DirectionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];
} IN;

vec3 SpecularLightColor = vec3(0.5, 0.5, 0.5);
float AmbientIntensity = 0.2;

float CalculateLightAttenuation(in vec3 lightPosition, in float intensity);
vec3 CalculateDirectionalLight(in vec3 viewDir, in vec3 lightPos, in vec3 normal, in vec3 lightColor);
vec3 CalculatePointLight(in PointLight light, in vec3 lightPos, in vec3 normal, in vec3 viewDir);

void main()
{
    
    if(IN.IsEmissive != 0)
    {
         FragColor = vec4(IN.Color.xyz,1);
         BloomColor = vec4(IN.Color.xyz,1);
    }
    else
    {
        vec4 texColor = IN.Color;
        vec3 toEye = normalize(u_cameraPos - IN.Position);
    
        vec3 color = vec3(0,0,0);

        for(int i = 0; i < u_directionalLightCount; i++)
        {
            vec3 lightDir = IN.DirectionalLightDirections[i];
            color += CalculateDirectionalLight(lightDir, toEye, IN.Normal,u_directionalLights[i].Color);
        }

        for(int i = 0; i < u_pointLightCount; i++)
        {
            color += CalculatePointLight(u_pointLights[i], u_pointLights[i].Position, IN.Normal, toEye);
        }
        
        vec3 ambient = texColor.rgb * AmbientIntensity;
        
        FragColor = vec4(ambient + color, 1.0);
        BloomColor = vec4(0,0,0,1);
    }
}

vec3 CalculateDirectionalLight(in vec3 lightDir, in vec3 viewDir, in vec3 normal, in vec3 lightColor)
{
    vec3 halfAngle = normalize(viewDir + lightDir);

    vec4 TexColor = IN.Color;

    vec3 ambient = AmbientIntensity * TexColor.rgb;
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * TexColor.rgb * lightColor;
    vec3 specular = SpecularLightColor * pow(max(0.0, dot(normal, halfAngle)), 1000);
    
    return (diffuse + specular);
}

float CalculateLightAttenuation(in vec3 lightPosition, float intensity, float range)
{
    float distance = length(lightPosition - IN.Position);

    float E = intensity;
    float DD = range * range;
    float Q = 1;
    float rr = distance * distance;

    return intensity * (DD / (DD + Q * rr));
}

vec3 CalculatePointLight(in PointLight light, in vec3 lightPos, in vec3 normal, in vec3 viewDir)
{
    vec3 lightDir = normalize(lightPos - IN.Position);
    vec3 halfAngle = normalize(viewDir + lightDir);

    vec4 TexColor = IN.Color;
    if(TexColor.a < 0.5)
        discard;

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * TexColor.rgb * light.Color;
    vec3 specular = pow(max(dot(normal, halfAngle), 0.0), 500) * SpecularLightColor;
    
    return CalculateLightAttenuation(lightPos, light.Intensity, light.Range) * diffuse + specular;
}