#include <stb_image/stb_image.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLTexture.hpp>
#include <Radiant/Rendering/Rendering.hpp>

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
			float* data = stbi_loadf(path.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
			RADIANT_VERIFY(data);
			imageSpec.Data = (std::byte*)data;
			imageSpec.Format = ImageFormat::RGBA16F;
		}
		else
		{
			RA_INFO("Loading texture {}, srgb = {}, HDR = {}", Utils::FileSystem::GetFileName(path), srgb, false);
			unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &nrChannels, srgb ? STBI_rgb : STBI_rgb_alpha);
			RADIANT_VERIFY(data);
			imageSpec.Data = (std::byte*)data;
			imageSpec.Format = srgb ? ImageFormat::RGBA : ImageFormat::RGB;
		}
		imageSpec.Width = width;
		imageSpec.Height = height;
		m_Image2D = Image2D::Create(imageSpec);

		Memory::Shared<Image2D>& image = m_Image2D;
		Rendering::SubmitCommand([image]() mutable
			{
				image.As<OpenGLImage2D>()->Invalidate();
			});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		m_Image2D.As<OpenGLImage2D>()->Release();
	}

	void OpenGLTexture2D::Use(uint32_t slot, BindUsage use) const
	{
		m_Image2D->Use(slot, use);
	}
}