#version 430 core

#define MAX_POINT_LIGHT_COUNT 18
#define MAX_DIRECTIONAL_LIGHT_COUNT 3
#define MAX_MAPS 5
#define PI 3.1415926538
#define NON_METAL_SURFACE_REFLECTION 0.04
#define TONE_MAPPING_COEFFICIENT 1
#define SHADOW_BIAS 0.0025

const vec3 fogPlaneNormal = vec3(0,1,0);

struct PointLightVertexData
{
	vec3 Position;
};

struct DirectionalLight
{
    vec3 Color;
    vec3 Direction;
    float Intensity;
};

struct PointLight
{
    vec3 Color;
    float Intensity;
    float Range;
};

struct MaterialData
{
    vec3 AlbedoFactor;
    vec3 EmissionFactor;
    float Metalness;
    float Roughness;

    sampler2D AlbedoTexture;
    sampler2D NormalTexture;
    sampler2D AOMROTexture;
    sampler2D EmissionTexture;
    sampler2D ShadowTexture;
};

uniform PointLight u_pointLights[MAX_POINT_LIGHT_COUNT];
uniform int u_pointLightCount;
uniform DirectionalLight u_directionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
uniform int u_directionalLightCount;

uniform vec3 u_cameraPos;
uniform vec3 u_fogColor;
uniform float u_fogDensity;

uniform MaterialData u_material;
uniform bool u_hasAlbedoTexture;
uniform bool u_hasNormalTexture;
uniform bool u_hasRoughnessMetalnessTexture;
uniform bool u_hasEmissiveTexture;
uniform bool u_useBakedAmbientOcclusion;
uniform bool u_flipNormals;
uniform bool u_hasVertexColors;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor;

in VS_OUT
{
	vec2 TexCoords;
    vec4 VertexColor;
    vec3 Position;
	vec3 TS_Position;
	vec3 TS_ViewDir;
	vec3 TS_DirectionalLightDirections[MAX_DIRECTIONAL_LIGHT_COUNT];
	vec3 TS_PointLightPositions[MAX_POINT_LIGHT_COUNT];
    vec4 LS_Position;

    float FogMagicValue1;
    float FogMagicValue2;
} IN;

float AmbientIntensity = 0.1;

vec3 CalculateCookTorranceBRDF(vec3 viewDir, vec3 lightDir, vec3 lightRadiance, vec3 surfReflection, vec3 normal, float metallic, float roughness);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 surfReflection);
float ShadowCalculation(vec4 fragPosLightSpace);

float CalculateLightAttenuation(in vec3 lightPosition, float intensity, float range);
vec3 ApplyHalfspaceFog(in vec3 PBRColor,in vec3 viewVectorUnnormalized, float planeVectorDotViewVector);

void main()
{
    //Vertex colors.
    vec3 vertexColor = u_hasVertexColors ? (int(u_hasVertexColors) * IN.VertexColor).rgb : vec3(1.0f);

    //Emission. (emission is not rendered using PBR)
    vec3 srgb = vec3(0);
    vec3 emissionValue = texture(u_material.EmissionTexture, IN.TexCoords).rgb;
    bool hasEmission = emissionValue.r > 0.0;
    if(hasEmission) 
    {
        srgb = emissionValue * u_material.EmissionFactor * vertexColor;
    }
    else
    {
        vec3 viewDir = normalize(IN.TS_ViewDir);
        vec3 albedo = u_hasAlbedoTexture 
            ? texture(u_material.AlbedoTexture, IN.TexCoords).rgb
            : u_material.AlbedoFactor;

        //Reading normal map.
        vec3 normal = vec3(0,0,1);
        if(u_hasNormalTexture)
        {
            normal = normalize(texture(u_material.NormalTexture, IN.TexCoords).rgb * 2.0 - 1.0);
            if(u_flipNormals) 
            {
                normal.y *= -1;
            }
        }

        //Reading metal/roughness/AO values.
        float ambientOcclusion,roughness,metallic;
        if(u_hasRoughnessMetalnessTexture)
        {
            vec3 PBRValue = texture(u_material.AOMROTexture, IN.TexCoords).rgb;
            ambientOcclusion = PBRValue.r;
            roughness = PBRValue.g;
            metallic = PBRValue.b;
        }
        else
        {
            ambientOcclusion = 1;
            roughness = u_material.Roughness;
            metallic = u_material.Metalness;
        }

        //Shadows.
        float shadow = ShadowCalculation(IN.LS_Position);

        //AO.
        vec3 ambient = u_useBakedAmbientOcclusion 
            ? vec3(AmbientIntensity * albedo * ambientOcclusion * (1.0 - shadow))
            : vec3(AmbientIntensity * albedo * (1.0 - shadow));

        vec3 surfaceReflection = vec3(0.04);
        surfaceReflection = mix(surfaceReflection, albedo, metallic);

        vec3 lo = vec3(0.0);

        //Directional light.
        for(int i = 0; i < u_directionalLightCount; i++)
        {
            vec3 lightDir = normalize(IN.TS_DirectionalLightDirections[i]);
            vec3 lightRadiance = u_directionalLights[i].Color * u_directionalLights[i].Intensity;
            lo += CalculateCookTorranceBRDF(viewDir, lightDir, lightRadiance, surfaceReflection, normal, metallic, roughness) * (1.0 - shadow);
        }

        //Point lights.
        for(int i = 0; i < u_pointLightCount; i++)
        {
            vec3 lightDir = normalize(IN.TS_PointLightPositions[i] - IN.TS_Position);
            float lightAttenuation = CalculateLightAttenuation(IN.TS_PointLightPositions[i], u_pointLights[i].Intensity, u_pointLights[i].Range);
            vec3 lightRadiance = u_pointLights[i].Color * lightAttenuation;
            lo += CalculateCookTorranceBRDF(viewDir, lightDir, lightRadiance, surfaceReflection, normal, metallic, roughness);
        }

        srgb = vertexColor * (ambient + lo);
    }

    BloomColor = hasEmission
        ? vec4(srgb, 1.0)
        : vec4(0.0, 0.0, 0.0, 1.0);

    vec3 UnNormalizedViewVector = u_cameraPos - IN.Position;

    FragColor = vec4(ApplyHalfspaceFog(srgb, UnNormalizedViewVector, dot(UnNormalizedViewVector,fogPlaneNormal)),1);//vec4(srgb, 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projectionCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float closestDepth = texture(u_material.ShadowTexture, projectionCoords.xy).r;
    float currentDepth = projectionCoords.z;

    if(projectionCoords.z > 1.0f)
    {
        return 0.0f;
    }

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_material.ShadowTexture, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_material.ShadowTexture, projectionCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - SHADOW_BIAS > pcfDepth ? 1.0 : 0.0;
        }
    }
       
    return (shadow / 9);
}


//Based on https://learnopengl.com/PBR/Lighting and changed around for better results.
vec3 CalculateCookTorranceBRDF(vec3 viewDir, vec3 lightDir, vec3 lightRadiance, vec3 surfReflection, vec3 normal, float metallic, float roughness)
{
    vec3 albedo;
    if(u_hasAlbedoTexture){
        albedo = texture(u_material.AlbedoTexture, IN.TexCoords).rgb;
    }else{
        albedo = u_material.AlbedoFactor;
    }
    vec3 halfway = normalize(viewDir + lightDir);

    float nDotV = max(dot(normal, viewDir), 0.0f);
    float nDotL = max(dot(normal, lightDir),0.0f);
    float hDotV = max(dot(halfway, viewDir), 0.0f);

    //Calculate the NDF and the Geometry function for the cook torrance BRDF.
    float ndf = DistributionGGX(normal, halfway, roughness);
    float g = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 f = FresnelSchlick(hDotV, surfReflection);

    //Calculate the light's contribution to the reflectance equation - which is what fresnel represents.
    vec3 reflectedEnergy = f;
    vec3 absorbedEnergy = vec3(1.0) - reflectedEnergy;
    absorbedEnergy *= 1.0 - metallic;

    //Calculate the diffuse and specular component.
    vec3 diffuse = absorbedEnergy * albedo / PI;
    vec3 numerator = ndf * g * f;
    float denominator = 4.0 * nDotV * nDotL;
    vec3 specular = numerator / max(denominator, 0.001);

    return (diffuse + specular) * lightRadiance * nDotL;
}

vec3 FresnelSchlick(float cosTheta, vec3 surfReflection)
{
    float c = 1.0 - cosTheta;
    return surfReflection + (1.0 - surfReflection) * (c * c * c * c * c); //don't sure POW, it's inefficient.
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float CalculateLightAttenuation(in vec3 lightPosition, float intensity, float range)
{
    float distance = length(lightPosition - IN.TS_Position);

    float E = intensity;
    float DD = range * range;
    float Q = 1;
    float rr = distance * distance;

    return intensity * (DD / (DD + Q * rr));
}

vec3 ApplyHalfspaceFog(in vec3 PBRColor,in vec3 viewVectorUnnormalized, float planeVectorDotViewVector)
{
    const float kFogEpsilon = 0.0001;

    float x = min(IN.FogMagicValue2,0.0);
    float tau = 0.5 * u_fogDensity *length(viewVectorUnnormalized) * (IN.FogMagicValue1 - x * x/ (abs(planeVectorDotViewVector) + kFogEpsilon));
    return (mix(u_fogColor,PBRColor,exp(tau)));
}