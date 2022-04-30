#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <Resources/Texture.h>
#include <memory>

namespace mrpoc
{
	/// <summary>
	/// The alpha value regarding the transparency in the material.
	/// </summary>
	enum class EAlphaMode
	{
		OPAQUE_MODE = 0,
		MASK_MODE,
		BLEND_MODE,

		COUNT
	};

	/// <summary>
	/// The material class follows a PBR metallic Roughness material approach.
	/// </summary>
	struct Material
	{
		Material();
		~Material() = default;

		glm::vec4 AlbedoFactor;
		glm::vec3 EmissionFactor;
		float MetallicFactor;
		float RoughnessFactor;

		std::string AlphaMode;
		float AlphaCutoff;

		float NormalScale;
		bool FlipNormals;

		bool HasOcclusionMap;
		bool DoubleSided;

		int TexCoords[static_cast<int>(ETextureType::Count)];

		std::weak_ptr<Texture> AlbedoMap;
		std::weak_ptr<Texture> MetalnessRoughnessMap;
		std::weak_ptr<Texture> NormalMap;
		std::weak_ptr<Texture> EmissionMap;

		//float m_occlusionStrength;
		//std::weak_ptr<Texture> m_occlusionMap; 
		
		//The occlusion map can (and should be) merge dwith the metalnessRoughness texture.
		//The 3 channel model is defined as: R = AO, G = Roughness, B = metallic. - blender also exports it this way, 
		//through the use of an in-editor export hack, because of prinicpled BRDF limitations: https://docs.blender.org/manual/en/latest/addons/import_export/scene_gltf2.html
	};
}