#pragma once

#include <filesystem>

namespace Radiant
{
	enum class RadiantShaderType
	{
		None = 0, 
		Vertex = 1,
		Fragment = 2,
		Compute = 3
	};

	class Shader : public Memory::RefCounted
	{
	public:
		virtual ~Shader() = default;

		virtual void Use() const = 0;
		virtual void Reload() = 0;

		static Memory::Shared<Shader> Create(const std::filesystem::path& path);
	};
}