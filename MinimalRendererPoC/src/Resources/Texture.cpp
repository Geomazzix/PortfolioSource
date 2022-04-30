#include "MrpocPch.h"
#include <Resources/Texture.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

namespace mrpoc
{
	void Texture::Initialize(const Image& image, std::weak_ptr<Sampler> sampler, std::vector<unsigned char>&& source, const std::string& name, int TextureFormat)
	{
		m_Image = image;

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLenum format;
		switch (image.Format)
		{
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		default:
			format = GL_RGBA;
			break;
		}

		GLenum type;
		switch (image.BitSize)
		{
		case 8:
			type = GL_UNSIGNED_BYTE;
			break;
		case 16:
			type = GL_UNSIGNED_SHORT;
			break;
		default:
			type = GL_UNSIGNED_INT;
			break;
		}

		if (!sampler.expired())
		{
			m_Sampler = sampler;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.lock()->MinFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.lock()->MagFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.lock()->WrapFilterS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.lock()->WrapFilterT);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_SRGB8_ALPHA8 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_SRGB8_ALPHA8 ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		}

		glTexImage2D(GL_TEXTURE_2D, 0, (TextureFormat != GL_SRGB8_ALPHA8 ? format : TextureFormat), image.Width, image.Height, 0, format, type, &source[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::Destroy()
	{
		glDeleteTextures(1, &m_id);
	}

	GLuint Texture::GetId() const
	{
		return m_id;
	}

	GLuint Texture::GetSlot() const
	{
		return m_slot;
	}

	void Texture::Bind(GLuint slot)
	{
		m_slot = slot;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(m_GLType, m_id);
	}

	void Texture::Unbind()
	{
		glBindTexture(m_GLType, 0);
		glActiveTexture(GL_TEXTURE0);
		m_slot = 0;
	}
}