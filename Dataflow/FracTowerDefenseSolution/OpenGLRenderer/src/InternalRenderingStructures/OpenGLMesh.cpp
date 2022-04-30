#include "OpenglRenderer/pch.h"
#include "OpenGLRenderer/InternalRenderingStructures/OpenGLMesh.h"

namespace TOR
{
	OpenGLMesh::OpenGLMesh() :
		m_VAO(0),
		m_EBO(0)
	{
	}

	void OpenGLMesh::BufferAttributeData(EVertexAttributes& attrType, uint32_t byteLength, uint32_t ByteStride, bool normalized, int attrElementCount, const void* bufferStart, int overwriteVertexAttribArrayPosition /*= -1*/, unsigned long DataType /*= GL_FLOAT*/, unsigned long DrawMode /*= GL_STATIC_DRAW*/)
	{
		glBindVertexArray(m_VAO);
		glGenBuffers(1, &m_VBO[(int)attrType]);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[(int)attrType]);
		glBufferData(GL_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);
		if (overwriteVertexAttribArrayPosition == -1) {
			glEnableVertexAttribArray((int)attrType + 3);
			glVertexAttribPointer((int)attrType + 3, attrElementCount, DataType, normalized, ByteStride, nullptr);
		}
		else
		{
			glEnableVertexAttribArray(overwriteVertexAttribArrayPosition);
			glVertexAttribPointer(overwriteVertexAttribArrayPosition, attrElementCount, DataType, normalized, ByteStride, nullptr);
		}
	}

	void OpenGLMesh::BufferInstanceAttributeData(EVertexAttributes attribType, uint32_t byteLength, const void* bufferStart, bool setNormalMats, unsigned long DrawMode /*= GL_STATIC_DRAW*/)
	{
		switch (attribType)
		{
		case EVertexAttributes::MODELTRANSFORM:
		{
			int indexModelTransform = static_cast<int>(EVertexAttributes::MODELTRANSFORM);
			glBindVertexArray(m_VAO);
			//glGenBuffers(1, &m_VBO[indexModelTransform]);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO[indexModelTransform]);
			glBufferData(GL_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);

			size_t sizeOfVec4 = sizeof(glm::vec4);

			glEnableVertexAttribArray(indexModelTransform);
			glVertexAttribPointer(indexModelTransform, 4, GL_FLOAT, GL_FALSE, 4 * sizeOfVec4, nullptr);

			glEnableVertexAttribArray(indexModelTransform + 1);
			glVertexAttribPointer(indexModelTransform + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeOfVec4, (void*)(sizeOfVec4 * 1));

			glEnableVertexAttribArray(indexModelTransform + 2);
			glVertexAttribPointer(indexModelTransform + 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeOfVec4, (void*)(sizeOfVec4 * 2));

			glEnableVertexAttribArray(indexModelTransform + 3);
			glVertexAttribPointer(indexModelTransform + 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeOfVec4, (void*)(sizeOfVec4 * 3));

			glVertexAttribDivisor(indexModelTransform, 1);
			glVertexAttribDivisor(indexModelTransform + 1, 1);
			glVertexAttribDivisor(indexModelTransform + 2, 1);
			glVertexAttribDivisor(indexModelTransform + 3, 1);
			break;
		}
		case EVertexAttributes::NORMALMATRIX:
		{
			int indexModelTransform = 9;
			//glBindVertexArray(m_VAO);
			//glGenBuffers(1, &m_VBO[indexModelTransform]);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO[static_cast<int>(EVertexAttributes::NORMALMATRIX)]);
			glBufferData(GL_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);

			size_t sizeOfVec3 = sizeof(glm::vec3);

			glEnableVertexAttribArray(indexModelTransform);
			glVertexAttribPointer(indexModelTransform, 3, GL_FLOAT, GL_FALSE, 3 * sizeOfVec3, nullptr);

			glEnableVertexAttribArray(indexModelTransform + 1);
			glVertexAttribPointer(indexModelTransform + 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeOfVec3, (void*)(sizeOfVec3 * 1));

			glEnableVertexAttribArray(indexModelTransform + 2);
			glVertexAttribPointer(indexModelTransform + 2, 3, GL_FLOAT, GL_FALSE, 3 * sizeOfVec3, (void*)(sizeOfVec3 * 2));

			glVertexAttribDivisor(indexModelTransform, 1);
			glVertexAttribDivisor(indexModelTransform + 1, 1);
			glVertexAttribDivisor(indexModelTransform + 2, 1);
			break;
		}
		}
	}

	void OpenGLMesh::BufferInstanceWildcardAttributeData(int locationOfAttribute, uint32_t byteLength, const void* bufferStart, int attrElementCount, unsigned long DrawMode /*= GL_STATIC_DRAW*/)
	{
		EVertexAttributes attribVBOToUse;
		if (m_attributeWildcardMap.find(locationOfAttribute) != m_attributeWildcardMap.end())
		{
			int wildCardNumber = m_attributeWildcardMap[locationOfAttribute];
			if (wildCardNumber == 0) attribVBOToUse = EVertexAttributes::WILDCARD_0;
			if (wildCardNumber == 1) attribVBOToUse = EVertexAttributes::WILDCARD_1;
			if (wildCardNumber == 2) attribVBOToUse = EVertexAttributes::WILDCARD_2;
		}
		else
		{
			if (m_wildcardUsedCount == 0)
			{
				attribVBOToUse = EVertexAttributes::WILDCARD_0;
				m_attributeWildcardMap[locationOfAttribute] = 0;
			}
			else if (m_wildcardUsedCount == 1)
			{
				attribVBOToUse = EVertexAttributes::WILDCARD_1;
				m_attributeWildcardMap[locationOfAttribute] = 1;
			}
			else if (m_wildcardUsedCount == 2)
			{
				attribVBOToUse = EVertexAttributes::WILDCARD_2;
				m_attributeWildcardMap[locationOfAttribute] = 2;
			}
			else {
				// No more than 3 wildcards allowed atm.
				TOR_ASSERT(false);
			}
			m_wildcardUsedCount++;
		}
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[(int)attribVBOToUse]);
		glBufferData(GL_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);

		glEnableVertexAttribArray(locationOfAttribute);
		glVertexAttribPointer(locationOfAttribute, attrElementCount, GL_FLOAT, GL_FALSE, 0, nullptr);

		glVertexAttribDivisor(locationOfAttribute, 1);
	}

	void OpenGLMesh::BufferElementData(uint32_t byteLength, void* bufferStart, unsigned int elementCount, unsigned long DrawMode /*= GL_DYNAMIC_DRAW*/)
	{
		// Index buffer
		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteLength, bufferStart, DrawMode);
		m_elementCount = elementCount == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		m_indexCount = byteLength / elementCount;
	}

	// if things go wrong and you can't find what it is. check if this VAO logic is correct on consequent function calls. as the bind vao call is new since new API code.
	void OpenGLMesh::Initialize()
	{
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);
		glGenBuffers(1, &m_VBO[static_cast<int>(EVertexAttributes::MODELTRANSFORM)]);
		glGenBuffers(1, &m_VBO[static_cast<int>(EVertexAttributes::NORMALMATRIX)]);
		glGenBuffers(1, &m_VBO[static_cast<int>(EVertexAttributes::WILDCARD_0)]);
		glGenBuffers(1, &m_VBO[static_cast<int>(EVertexAttributes::WILDCARD_1)]);
		glGenBuffers(1, &m_VBO[static_cast<int>(EVertexAttributes::WILDCARD_2)]);
	}

	void OpenGLMesh::Destroy()
	{
		glDeleteVertexArrays(1, &m_VAO);
	}

	void OpenGLMesh::Bind() const
	{
		glBindVertexArray(m_VAO);
	}

	void OpenGLMesh::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLMesh::SetShader(std::shared_ptr<OpenGLShader> shader)
	{
		m_shader = shader;
	}

	std::shared_ptr<OpenGLShader> OpenGLMesh::GetShader()
	{
		return m_shader;
	}

	void OpenGLMesh::Draw()
	{
		glDrawElements(GL_TRIANGLES, m_indexCount, m_elementCount, nullptr);
	}

	void OpenGLMesh::DrawInstanced(unsigned instanceCount)
	{
		glDrawElementsInstanced(GL_TRIANGLES, m_indexCount, m_elementCount, nullptr, instanceCount);
	}
}