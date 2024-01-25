#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::filesystem::path& path, bool srgb);
		virtual ~OpenGLTexture2D() override;

		virtual void Use(uint32_t slot = 0, BindUsage use = BindUsage::Bind) const override;

		virtual const std::filesystem::path& GetPath() const override { return m_FilePath; }
		virtual const std::string& GetName() const override { return m_Name; }
		virtual bool Loaded() const override { return m_Loaded; }

		virtual const Memory::Shared<Image2D>& GetImage2D() const override { return m_Image2D; }
	private:
		std::filesystem::path m_FilePath;
		RenderingID m_RenderingID = 0;
		std::string m_Name;
		bool m_sRGB;
		Memory::Shared<Image2D> m_Image2D;
		bool m_Loaded = false;
	};
}