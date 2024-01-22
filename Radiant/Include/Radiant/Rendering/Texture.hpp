#pragma once

#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
	class Texture : public Memory::RefCounted
	{
	public:
		virtual ~Texture() = default;

		virtual void Use(uint32_t slot, BindUsage use) const = 0;

		virtual const std::filesystem::path& GetPath() const = 0;
		virtual const std::string& GetName() const = 0;
		virtual bool Loaded() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual const Memory::Shared<Image2D>& GetImage2D() const = 0;

		Memory::Shared<Texture2D> Create(const std::filesystem::path& path, bool srgb = false);
	};
}