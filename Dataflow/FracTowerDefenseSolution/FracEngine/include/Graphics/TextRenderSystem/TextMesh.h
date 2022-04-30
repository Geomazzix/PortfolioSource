#pragma once
#include <vector>

#include "Graphics/TextRenderSystem/TextTexture.h"

namespace Frac
{
	struct Vertex;
	
	/*!
	* @brief TEMP CLASS FOR TEXT MESH UNTIL RENDERCORE IS READY
	*/
	struct TextMesh
	{
		unsigned int m_vao;
		unsigned int m_vbo;
		std::vector<Frac::Vertex> m_verts;
		Frac::TextTexture m_texture;
		bool m_useTexture = true;
	};
}
