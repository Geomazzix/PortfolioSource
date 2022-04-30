#pragma once
#include <glm/glm.hpp>

namespace mrpoc
{
	enum Attributes 
	{
		POSITION = 0,
		NORMAL,
		TANGENT,
		TEXCOORD_0,
		TEXCOORD_1,
		COLOR_0,
		JOINTS_0,
		WEIGHTS_0
	};

	/// <summary>
	/// The vertex attributes are defined within the vertex.
	/// </summary>
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec2 TexCoord_0;
		glm::vec2 TexCoord_1;
		glm::vec4 Color_0;

		Vertex(const glm::vec3& Position,
			const glm::vec3& Normal,
			const glm::vec3& Tangent,
			const glm::vec2& TexCoord_0,
			const glm::vec2& TexCoord_1,
			const glm::vec4& Color_0);
		~Vertex() = default;
		
		bool operator==(const Vertex& rhs) const;
	};
}