#version 430 core

out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoords;
} IN;

uniform sampler2D u_pbrPass;
uniform sampler2D u_bloomPass;
uniform float u_exposure;
uniform float u_gamma;

uniform int u_mipLevels;
uniform int u_startMipLevel;
uniform float u_bloomIntensity;

void main()
{
	vec3 srgb = texture(u_pbrPass, IN.TexCoords).rgb;
	vec3 bloomSrgb;
	float weight = 1.0 / ((u_mipLevels + 1) - u_startMipLevel);

	for(int i = u_startMipLevel; i < u_mipLevels + 1; ++i)
	{
		bloomSrgb = textureLod(u_bloomPass, IN.TexCoords, i).rgb;
		srgb += bloomSrgb * u_bloomIntensity;
	}

	//Gamma correction.
    vec3 mapped = vec3(1.0) - exp(-srgb * u_exposure);
    vec3 rgb = pow(mapped, vec3(1.0 / u_gamma));
	FragColor = vec4(rgb, 1.0);
}