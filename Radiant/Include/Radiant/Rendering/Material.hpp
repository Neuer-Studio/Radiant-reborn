#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Rendering/Texture.hpp>

namespace Radiant
{
	class Material : public Memory::RefCounted
	{
	public:
		virtual ~Material() = default;
		virtual void Use() const = 0;

		virtual void SetUniform(const std::string& bufferName, const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetUniform(const std::string& bufferName, const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetUniform(const std::string& bufferName, const std::string& name, const glm::mat4& value) const = 0;
		virtual void SetUniform(const std::string& bufferName, const std::string& name, float value) const = 0;
		virtual void SetUniform(const std::string& bufferName, const std::string& name, bool value) const = 0;
		virtual void SetUniform(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const = 0;

		static Memory::Shared<Material> Create(const Memory::Shared<Shader>& shader);
	};
}