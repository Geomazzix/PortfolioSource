#pragma once

#include <Graphics/Vertex.h>
#include <vector>
#include <string>
#include <memory>
#include <array>

namespace mrpoc
{
	class Shader;
	class Material;

	/// <summary>
	/// A mesh represents a data structure that combines part of a models geometry to its visual representation.
	/// </summary>
	class Mesh
	{
	public:
		Mesh();
		~Mesh() = default;

		//void Initialize(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indicies, GLenum glDrawType);
		//void Initialize(GLuint vao, GLuint vbo, GLuint vertexCount, GLuint ebo, GLuint indiciesCount);

		void BufferAttributeData(Attributes& attrType, uint32_t byteLength, uint32_t ByteStride, int attrElementCount, const void* bufferStart, unsigned long DataType = GL_FLOAT, unsigned long DrawMode = GL_STATIC_DRAW);
		void BufferElementData(uint32_t byteLength, void* bufferStart, unsigned long DrawMode = GL_DYNAMIC_DRAW);

		void Destroy();

		void Bind() const;
		void Unbind() const;

		void SetShader(std::shared_ptr<Shader> shader);
		std::shared_ptr<Shader> GetShader();

		void Draw();

		GLuint GetVertexCount() const { return m_vertexCount; }
		void SetVertexCount(GLuint valueToSet) { m_vertexCount = valueToSet; }
		GLuint GetIndiceCount() const { return m_indiceCount; }
		void SetIndiceCount(int valueToSet) { m_indiceCount = valueToSet; }
		GLenum GetElementDataType() const { return m_elementDataType; }
		void SetElementDataType(GLenum valueToSet) { m_elementDataType = valueToSet; }
		std::shared_ptr<Material> GetMaterial() const { return m_material; }
		void SetMaterial(std::shared_ptr<Material> valueToSet) { m_material = valueToSet; }
	private:
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Material> m_material;

		GLuint m_VAO;
		std::array<GLuint, 8> m_VBO;
		GLuint m_EBO;

		GLenum m_elementDataType;

		GLuint m_vertexCount;
		GLuint m_indiceCount;
	};
}