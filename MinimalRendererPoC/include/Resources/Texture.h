#pragma once

namespace mrpoc
{
	/// <summary>
	/// Contains all specific image property data.
	/// </summary>
	struct Image
	{
		std::string FilePath;
		int Width;
		int Height;
		int Format;
		int BitSize;
	};

	/// <summary>
	/// Contains the properties for the wrapping of the texture.
	/// </summary>
	struct Sampler
	{
		int MinFilter;
		int MagFilter;
		int WrapFilterS;
		int WrapFilterT;
		int WrapFilterR;
	};

	enum class ETextureType {
		Albedo = 0,
		MetalnessRoughness,
		Normal,
		Occlusion,
		Emission,
		Count
	};

	/// <summary>
	/// The texture class contains all the pointers required to access the texture data on the GPU.
	/// </summary>
	class Texture
	{
	public:
		Texture() = default;
		~Texture() = default;

		void Initialize(const Image& image, std::weak_ptr<Sampler> sampler, std::vector<unsigned char>&& Source, const std::string& name, int TextureFormat = GL_RGBA);
		void Destroy();

		GLuint GetId() const;
		GLuint GetSlot() const;

		void Bind(GLuint slot);
		void Unbind();

	private:
		GLuint m_id;
		GLuint m_slot;
		GLenum m_GLType = GL_TEXTURE_2D;
		Image m_Image;

		std::weak_ptr<Sampler> m_Sampler;
	};
}