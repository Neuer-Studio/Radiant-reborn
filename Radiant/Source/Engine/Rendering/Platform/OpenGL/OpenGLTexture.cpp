#include <stb_image/stb_image.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLTexture.hpp>

namespace Radiant
{

	OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path, bool srgb)
		: m_FilePath(path), m_Name(Utils::FileSystem::GetFileName(path)), m_sRGB(srgb)
	{
		ImageSpecification imageSpec = {};
		int width, height, nrChannels;
		if (stbi_is_hdr(path.string().c_str()))
		{
			RA_INFO("Loading HDR texture {}, srgb = {}, HDR = {}", Utils::FileSystem::GetFileName(path), srgb, true);
			RADIANT_VERIFY(false);
		}
		else
		{
			RA_INFO("Loading texture {}, srgb = {}, HDR = {}", Utils::FileSystem::GetFileName(path), srgb, false);
			unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, srgb ? STBI_rgb : STBI_rgb_alpha);
		}
		imageSpec.Width = width;
		imageSpec.Height = height;
		m_Image2D = OpenGLImage2D::Create();
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		m_Image2D->Release();
	}

	void OpenGLTexture2D::Use(uint32_t slot, BindUsage use) const
	{
		m_Image2D->Use(slot, use);
	}
}