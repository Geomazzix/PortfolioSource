#include "MrpocPch.h"
#include <Graphics/Vertex.h>

namespace mrpoc
{
	Vertex::Vertex(const glm::vec3& Position,
	const glm::vec3& Normal,
	const glm::vec3& Tangent,
	const glm::vec2& TexCoord_0,
	const glm::vec2& TexCoord_1,
	const glm::vec4& Color_0) :
		Position(Position), Normal(Normal), Tangent(Tangent), TexCoord_0(TexCoord_0), TexCoord_1(TexCoord_1), Color_0(Color_0)
	{}

	bool Vertex::operator==(const Vertex& rhs) const
	{
		return Position == rhs.Position && Color_0 == rhs.Color_0 && TexCoord_0 == rhs.TexCoord_0;
	}
}