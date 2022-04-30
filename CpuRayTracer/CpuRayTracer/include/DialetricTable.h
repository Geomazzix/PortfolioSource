#pragma once
#include <unordered_map>

namespace CRT
{
	/// <summary>
	/// The different types of refractive media that are currently stored in the renderer.
	/// </summary>
	enum class EDialetricType
	{
		AIR,
		GLASS,
		WATER
	};

	/// <summary>
	/// Holds the index of different refractive media.
	/// https://refractiveindex.info
	/// </summary>
	class DialetricIndexTable
	{
	public:
		DialetricIndexTable() = default;
		~DialetricIndexTable();

		static float GetDialetricIndex(EDialetricType type);

	private:
		static std::unordered_map<EDialetricType, float> m_DialetricIndices;
	};
}