#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Rendering/RenderingTypes.hpp>

namespace Radiant
{
	enum class MaterialFlag
	{
		None = BIT(0),
		DepthTest = BIT(1),
		Blend = BIT(2),
		TwoSided = BIT(3)
	};

	class Material : public Memory::RefCounted
	{
	public:
		virtual ~Material() = default;
		virtual void Use() const = 0;

		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::mat4& value) const = 0;
		virtual void SetUBO(BindingPoint binding, const std::string& name, float value) const = 0;
		virtual void SetUBO(BindingPoint binding, const std::string& name, bool value) const = 0;
		virtual void SetUBO(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const = 0;
		virtual void SetUBO(const std::string& name, const Memory::Shared<Image2D>& image2D) const = 0;

		virtual void LoadUniformToBuffer(const std::string& name, RadiantShaderType type, RadiantShaderDataType dataType) const = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) const = 0;

		static Memory::Shared<Material> Create(const Memory::Shared<Shader>& shader);

		uint32_t GetFlags() const { return m_MaterialFlags; }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value = true)
		{
			if (value)
			{
				m_MaterialFlags |= (uint32_t)flag;
			}
			else
			{
				m_MaterialFlags &= ~(uint32_t)flag;
			}

		}
	private:
		uint32_t m_MaterialFlags;
	};
}