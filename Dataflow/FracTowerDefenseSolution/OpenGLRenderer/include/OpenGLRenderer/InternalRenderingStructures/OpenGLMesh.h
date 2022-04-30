#pragma once
#include <CoreRenderAPI/Components/Mesh.h>
#include <CoreRenderAPI/Components/Material.h>
#include <OpenGLRenderer/InternalRenderingStructures/OpenGLShader.h>

namespace TOR
{
	class OpenGLMesh
	{
	public:
		OpenGLMesh();
		~OpenGLMesh() = default;

		void Initialize();
		void Destroy();

		void BufferAttributeData(EVertexAttributes& attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, int overwriteVertexAttribArrayPosition = -1, unsigned long DataType = GL_FLOAT, unsigned long DrawMode = GL_STATIC_DRAW);
		void BufferInstanceAttributeData(EVertexAttributes attribType, uint32_t byteLength, const void* bufferStart, bool setNormalMats, unsigned long DrawMode = GL_STATIC_DRAW);
		void BufferInstanceWildcardAttributeData(int locationOfAttribute, uint32_t byteLength, const void* bufferStart, int attrElementCount, unsigned long DrawMode = GL_STATIC_DRAW);
		void BufferElementData(uint32_t byteLength, void* bufferStart, unsigned int elementCount, unsigned long DrawMode = GL_DYNAMIC_DRAW);

		void Bind() const;
		void Unbind() const;

		void SetShader(std::shared_ptr<OpenGLShader> shader);
		std::shared_ptr<OpenGLShader> GetShader();

		void Draw();
		void DrawInstanced(unsigned instanceCount);

	private:
		std::shared_ptr<OpenGLShader> m_shader;

		GLuint m_VAO;
		std::array<GLuint, static_cast<unsigned>(EVertexAttributes::COUNT)> m_VBO;
		std::map<int, int> m_attributeWildcardMap;
		GLuint m_EBO;
		unsigned long m_elementCount;
		unsigned int m_indexCount;

		int m_wildcardUsedCount = 0;
	};
}