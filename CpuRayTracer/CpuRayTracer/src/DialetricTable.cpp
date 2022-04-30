#include "DialetricTable.h"

namespace CRT
{
	std::unordered_map<EDialetricType, float> DialetricIndexTable::m_DialetricIndices =
	{
		{ EDialetricType::AIR, 1.00027717f },
		{ EDialetricType::GLASS, 1.5168f},
		{ EDialetricType::WATER, 1.3325f }
	};

	DialetricIndexTable::~DialetricIndexTable()
	{
		m_DialetricIndices.clear();
	}

	float DialetricIndexTable::GetDialetricIndex(EDialetricType type)
	{
		return m_DialetricIndices[type];
	}
}