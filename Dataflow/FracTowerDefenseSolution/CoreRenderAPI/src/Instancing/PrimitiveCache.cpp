#include "CoreRenderAPI/pch.h"
#include "CoreRenderAPI/Instancing/PrimitiveCache.h"

#include "CoreRenderAPI/Components/Mesh.h"
#include "CoreRenderAPI/LowLevelAPI.h"

namespace TOR
{
	void CalculateTangentArray(long vertexCount, const glm::vec3* vertex, const glm::vec3* normal,
		const glm::vec2* texcoord, long triangleCount, const int* indexes, glm::vec4* tangent)
	{
		glm::vec3* tan1 = new glm::vec3[vertexCount * 2]{ glm::vec3(0,0,0) };
		glm::vec3* tan2 = tan1 + vertexCount;

		for (long a = 0; a < triangleCount; a++)
		{
			long i1 = indexes[0];
			long i2 = indexes[1];
			long i3 = indexes[2];

			const glm::vec3& v1 = vertex[i1];
			const glm::vec3& v2 = vertex[i2];
			const glm::vec3& v3 = vertex[i3];

			const glm::vec2& w1 = texcoord[i1];
			const glm::vec2& w2 = texcoord[i2];
			const glm::vec2& w3 = texcoord[i3];

			float x1 = v2.x - v1.x;
			float x2 = v3.x - v1.x;
			float y1 = v2.y - v1.y;
			float y2 = v3.y - v1.y;
			float z1 = v2.z - v1.z;
			float z2 = v3.z - v1.z;

			float s1 = w2.x - w1.x;
			float s2 = w3.x - w1.x;
			float t1 = w2.y - w1.y;
			float t2 = w3.y - w1.y;

			float r = 1.0F / (s1 * t2 - s2 * t1);
			glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r);
			glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r);

			tan1[i1] += sdir;
			tan1[i2] += sdir;
			tan1[i3] += sdir;

			tan2[i1] += tdir;
			tan2[i2] += tdir;
			tan2[i3] += tdir;

			indexes += 3;
		}

		for (long a = 0; a < vertexCount; a++)
		{
			const glm::vec3& n = normal[a];
			const glm::vec3& t = tan1[a];

			// Gram-Schmidt orthogonalize
			tangent[a] = glm::vec4(glm::normalize(t - n * glm::dot(n, t)), 0);

			// Calculate handedness
			tangent[a].w = (glm::dot(glm::cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
		}

		delete[] tan1;
	}

	PrimitiveCache::PrimitiveCache()
	{
	}

	void PrimitiveCache::Initialize(LowLevelAPI* in_lowLevelRenderAPI)
	{
		InitializeCubePrimitive(in_lowLevelRenderAPI);
	}

	void PrimitiveCache::ShutDown(LowLevelAPI* in_lowLevelRenderAPI)
	{
		// Clean up cube
		in_lowLevelRenderAPI->DestroyMesh(m_PrimitiveCache[static_cast<int>(PrimitiveShape::CUBE)].data.Id);
	}

	const TOR::Primitive& PrimitiveCache::GetPrimitive(PrimitiveShape in_primitiveType) const
	{
		return m_PrimitiveCache[static_cast<int>(in_primitiveType)];
	}

	void PrimitiveCache::InitializeCubePrimitive(LowLevelAPI* in_lowLevelRenderAPI)
	{
		glm::vec3 vertices[8] =
		{
			glm::vec3(-1, -1, -1),
			glm::vec3(1, -1, -1),
			glm::vec3(1, 1, -1),
			glm::vec3(-1, 1, -1),
			glm::vec3(-1, -1, 1),
			glm::vec3(1, -1, 1),
			glm::vec3(1, 1, 1),
			glm::vec3(-1, 1, 1)
		};

		glm::vec2 texCoords[8] =
		{
			glm::vec2(0, 0),
			glm::vec2(1, 0),
			glm::vec2(1, 1),
			glm::vec2(0, 1)
		};

		glm::vec3 normals[6] =
		{
			glm::vec3(0, 0, 1),
			glm::vec3(1, 0, 0),
			glm::vec3(0, 0, -1),
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(0, -1, 0)
		};

		int indices[6 * 6] =
		{
			0, 1, 3, 3, 1, 2,
			1, 5, 2, 2, 5, 6,
			5, 4, 6, 6, 4, 7,
			4, 0, 7, 7, 0, 3,
			3, 2, 7, 7, 2, 6,
			4, 5, 0, 0, 5, 1
		};

		int texInds[6] = { 0, 1, 3, 3, 1, 2 };

		glm::vec3 vertexBuffer[36];
		for (int i = 0; i < 36; i++) {
			vertexBuffer[i] = glm::vec3(vertices[indices[i]]);
		}

		//glm::vec2 textureBuffer[36];
		//for (int i = 0; i < 36; i++) {
		//	textureBuffer[i] = texCoords[texInds[i % 4]];
		//}

		glm::vec3 normalBuffer[36];
		for (int i = 0; i < 36; i++) {
			normalBuffer[i] = glm::vec3(normals[indices[i / 6]]);
		}

		//glm::vec4 tangentBuffer[36];
		//CalculateTangentArray(36, vertices, normalBuffer, textureBuffer, 12, indices, tangentBuffer);

		//glm::vec3 colorBuffer[36]{ glm::vec3(0,0,0) };

		Mesh CubeMesh;
		in_lowLevelRenderAPI->CreateMesh(CubeMesh);

		in_lowLevelRenderAPI->BindMeshForRecording(CubeMesh.Id);
		// Buffer attribute Data
		in_lowLevelRenderAPI->BufferAttributeData(CubeMesh.Id, TOR::EVertexAttributes::POSITION, sizeof(glm::vec3) * 8, 0, false, 3, vertices, 4);
		in_lowLevelRenderAPI->BufferAttributeData(CubeMesh.Id, TOR::EVertexAttributes::NORMAL, sizeof(glm::vec3) * 36, 0, false, 3, normalBuffer, 5);
		//in_lowLevelRenderAPI->BufferAttributeData(CubeMesh.Id, TOR::EVertexAttributes::TEXCOORD_0, sizeof(glm::vec2) * 36, 0, false, 3, textureBuffer);
		//in_lowLevelRenderAPI->BufferAttributeData(CubeMesh.Id, TOR::EVertexAttributes::TANGENT, sizeof(glm::vec4) * 36, 0, false, 3, tangentBuffer);
		//in_lowLevelRenderAPI->BufferAttributeData(CubeMesh.Id, TOR::EVertexAttributes::COLOR_0, sizeof(glm::vec3) * 36, 0, false, 3, colorBuffer);

		// Buffer element data
		in_lowLevelRenderAPI->BufferElementData(CubeMesh.Id, sizeof(float) * 36, indices, 4);
		in_lowLevelRenderAPI->AssignShaderToMesh(CubeMesh.Id, TOR::EShaderType::Particle);
		// unbind
		in_lowLevelRenderAPI->UnbindMesh(CubeMesh.Id);

		m_PrimitiveCache[static_cast<int>(PrimitiveShape::CUBE)] = Primitive(PrimitiveShape::CUBE, CubeMesh);
	}

	void PrimitiveCache::InitializeSpherePrimitive(LowLevelAPI* in_lowLevelRenderAPI)
	{
	}

	void PrimitiveCache::InitializePlanePrimitive(LowLevelAPI* in_lowLevelRenderAPI)
	{
	}
}