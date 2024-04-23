#pragma once

#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	struct Texture2DCreateInformation
	{
		Memory::Buffer Buffer;
		std::string Name;
		uint32_t Width;
		uint32_t Height;
		ImageFormat Format;
	};

	class Texture : public Memory::RefCounted
	{
	public:
		virtual ~Texture() = default;

		virtual void Use(uint32_t slot = 0, BindUsage use = BindUsage::Bind) const = 0;

		virtual const std::filesystem::path& GetPath() const = 0;
		virtual const std::string& GetName() const = 0;
		virtual bool Loaded() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual const Memory::Shared<Image2D>& GetImage2D() const = 0;

		static Memory::Shared<Texture2D> Create(const std::filesystem::path& path, bool srgb = false);
		static Memory::Shared<Texture2D> Create(const Texture2DCreateInformation& info);
	};
}