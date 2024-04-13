#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Rendering/RenderingTypes.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	enum class MaterialFlag
	{
		None = BIT(0),
		DepthTest = BIT(1),
		Blend = BIT(2),
		TwoSided = BIT(3)
	};

	// TODO: Change flow(pass the value to buffer and via UpdateForRendering() set the value) 
	class Material : public Memory::RefCounted
	{
	public:
		virtual ~Material() = default;
		virtual void Use() const = 0;

		static void SetUBO(BindingPoint binding, const std::string& name, const glm::vec3& value); //TODO: static
		static void SetUBO(BindingPoint binding, const std::string& name, const glm::vec2& value);
		static void SetUBO(BindingPoint binding, const std::string& name, const glm::mat4& value);
		static void SetUBO(BindingPoint binding, const std::string& name, float value);
		static void SetUBO(BindingPoint binding, const std::string& name, bool value);
		static void SetUBO(BindingPoint binding, const std::string& name, const void* data, std::size_t size); // NOTE: Using for update structs
		virtual void SetImage2D(const std::string& name, const Memory::Shared<Texture2D>& texture2D, uint32_t sampler = 0)const = 0; //TODO: sampler -> std::optional
		virtual void SetImage2D(const std::string& name, const Memory::Shared<Image2D>& image2D, uint32_t sampler = 0) const = 0;

		virtual void UpdateForRendering() const = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) const = 0;
		virtual void SetBool(const std::string& name, bool value) const = 0;
		virtual void SetUint(const std::string& name, uint32_t value) const = 0;
		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetVec3(const std::string& name, const glm::vec3 value) const = 0;
		virtual void SetVec4(const std::string& name, const glm::vec4 value) const = 0;

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