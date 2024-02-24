#pragma once

#include <Radiant/Rendering/RenderingTypes.hpp>

namespace Radiant
{
	enum class ImageFormat
	{
		None = 0,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
	};

	enum class TextureRendererType
	{
		Texture2D = 0,
		TextureCube,
	};

	struct ImageSpecification
	{
		uint32_t Width;
		uint32_t Height;
		ImageFormat Format;
		TextureRendererType Type;
		std::byte* Data;
	};

	class Image : public Memory::RefCounted
	{
	public:
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual ImageFormat GetImageFormat() const = 0;
		virtual RenderingID GetTextureID() const = 0;
		virtual uint32_t GetMipmapLevels() const = 0;
		virtual void Use(uint32_t slot = 0, BindUsage use = BindUsage::Bind) const = 0;
	};

	class Image2D : public Image
	{
	public:

		static Memory::Shared<Image2D> Create(const ImageSpecification& spec);
	};

}