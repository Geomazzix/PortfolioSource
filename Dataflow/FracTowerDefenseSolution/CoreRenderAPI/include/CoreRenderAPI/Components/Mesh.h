#pragma once
namespace TOR
{
	enum class EShaderType
	{
		Diffuse = 0,
		DiffusePBR,
		DiffuseWithNormalMapping,
		ShadowMapping,
		gaussianBlur,
		PostProcessFinal,
		Particle,

		COUNT
	};

	enum class ETorDataType
	{
		BYTE = 0x1400,
		UBYTE = 0x1401,
		SHORT = 0x1402,
		USHORT = 0x1403,
		INT = 0x1404,
		UINT = 0x1405,
		FLOAT = 0x1406,
		BYTES_2 = 0x1407,
		BYTES_3 = 0x1408,
		BYTES_4 = 0x1409,
		DOUBLE = 0x140A
	};

	enum class EVertexAttributes
	{
		MODELTRANSFORM = 0,
		POSITION,
		NORMAL,
		TANGENT,
		TEXCOORD_0,
		COLOR_0,
		NORMALMATRIX,
		TEXCOORD_1,
		JOINTS_0,
		WEIGHTS_0,
		WILDCARD_0,
		WILDCARD_1,
		WILDCARD_2,
		COUNT
	};

	struct Mesh
	{
		unsigned Id;

		unsigned VertexCount;
		unsigned IndiceCount;

		ETorDataType ElementDataType;
	};

	inline bool operator<(const Mesh& lhs, const Mesh& rhs)
	{
		return lhs.Id < rhs.Id;
	}
}