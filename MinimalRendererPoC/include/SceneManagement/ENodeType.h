#pragma once

namespace mrpoc
{
	/// <summary>
	/// Defines the type of node used in the scene graph.
	/// </summary>
	enum ENodeType
	{
		CAMERA = 0,
		LIGHT = 1,
		MODEL = 2,
		ROOT = 3,
		EMPTY = 4,

		COUNT
	};
}