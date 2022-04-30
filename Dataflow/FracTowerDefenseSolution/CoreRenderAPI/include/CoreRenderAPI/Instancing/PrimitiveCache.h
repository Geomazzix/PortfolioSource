#pragma once
#include "CoreRenderAPI/Components/Mesh.h"

namespace TOR
{
	class LowLevelAPI;

	enum class PrimitiveShape {
		CUBE = 0,
		//SPHERE,
		//QUAD,
		COUNT
	};

	struct Primitive {
		Primitive() { shape = PrimitiveShape::CUBE; };
		Primitive(PrimitiveShape in_shape, const Mesh& in_data) : shape(in_shape), data(in_data) {}

		PrimitiveShape shape;
		Mesh data;
	};

	class PrimitiveCache {
	public:

		PrimitiveCache();
		~PrimitiveCache() = default;

		// We're not going to copy a primitive cache
		PrimitiveCache(const PrimitiveCache&) = delete;
		PrimitiveCache& operator=(const PrimitiveCache&) = delete;

		void Initialize(LowLevelAPI* in_lowLevelRenderAPI);
		void ShutDown(LowLevelAPI* in_lowLevelRenderAPI);

		const Primitive& GetPrimitive(PrimitiveShape in_primitiveType) const;

	private:
		std::array<Primitive, static_cast<int>(PrimitiveShape::COUNT)> m_PrimitiveCache;

		void InitializeCubePrimitive(LowLevelAPI* in_lowLevelRenderAPI);
		void InitializeSpherePrimitive(LowLevelAPI* in_lowLevelRenderAPI);
		void InitializePlanePrimitive(LowLevelAPI* in_lowLevelRenderAPI);
	};
}