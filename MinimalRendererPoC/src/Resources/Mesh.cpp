#include "MrpocPch.h"
#include <Resources/Mesh.h>

#include <Resources/Shader.h>

namespace mrpoc
{
	Mesh::Mesh() : 
		m_VAO(0),  
		m_EBO(0), 
		m_vertexCount(0),
		m_indiceCount(0)
	{
		glGenVertexArrays(1, &m_VAO);
	}

	//void Mesh::Initialize(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indicies, GLenum glDrawType)
	//{
		/*m_vertexCount = vertices.size();
		m_indiceCount = indicies.size();

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(1, &m_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), glDrawType);

		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(GLuint), indicies.data(), glDrawType);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Position)));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);*/
	//}

	//void Mesh::Initialize(GLuint vao, GLuint vbo, GLuint vertexCount, GLuint ebo, GLuint indiciesCount)
	//{
		/*m_VBO = vbo;
		m_EBO = ebo;

		m_vertexCount = vertexCount;
		m_indiceCount = indiciesCount;


		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		glBindVertexArray(0);*/
	//}

	void Mesh::BufferAttributeData(Attributes& attrType, uint32_t byteLength, uint32_t ByteStride, int attrElementCount, const void* bufferStart, unsigned long DataType /*= GL_FLOAT*/, unsigned long DrawMode /*= GL_STATIC_DRAW*/)
	{
		glBindVertexArray(m_VAO);
		glGenBuffers(1, &m_VBO[(int)attrType]);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[(int)attrType]);
		glBufferData(GL_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);
		glEnableVertexAttribArray((int)attrType);
		
		glVertexAttribPointer((int)attrType, attrElementCount, DataType, GL_FALSE, ByteStride, nullptr);
	}

	void Mesh::BufferElementData(uint32_t byteLength, void* bufferStart, unsigned long DrawMode /*= GL_DYNAMIC_DRAW*/)
	{
		// Index buffer
		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);
	}

	void Mesh::Destroy()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}

	void Mesh::Bind() const
	{
		glBindVertexArray(m_VAO);
	}

	void Mesh::Unbind() const
	{
		glBindVertexArray(0);
	}

	void Mesh::SetShader(std::shared_ptr<Shader> shader)
	{
		m_shader = shader;
	}

	std::shared_ptr<Shader> Mesh::GetShader()
	{
		return m_shader;
	}

	void Mesh::Draw()
	{
		glDrawElements(GL_TRIANGLES, GetIndiceCount(), GL_UNSIGNED_SHORT, nullptr);
	}
}