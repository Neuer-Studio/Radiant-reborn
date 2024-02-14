#pragma once

#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
	class OpenGLImage2D : public Image2D
	{
	public:
		OpenGLImage2D(const ImageSpecification& spec);

		virtual uint32_t GetWidth() const override { return m_Specification.Width; }
		virtual uint32_t GetHeight() const override { return m_Specification.Height; }
		virtual ImageFormat GetImageFormat() const override { return m_Specification.Format; }
		virtual RenderingID GetTextureID() const override { return m_RenderingID.value_or(0); }
		virtual uint32_t GetMipmapLevels() const override { return m_MipmapLevels; }

		virtual void Use(uint32_t slot = 0, BindUsage use = BindUsage::Bind) const override;
	public:
		void Invalidate();
		void Release();
	private:
		ImageSpecification m_Specification;
		std::optional<RenderingID> m_RenderingID;
		uint32_t m_MipmapLevels = 0;
	};
}