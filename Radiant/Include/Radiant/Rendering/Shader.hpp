#pragma once

#include <glm/glm.hpp>
#include <Radiant/Rendering/RenderingTypes.hpp>

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

	enum class RadiantShaderDataType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		UInt,
		Mat3, Mat4,
		Bool
	};

	enum class RadiantShaderSamplerDataType
	{
		None = 0,
		Sampler1D, Sampler2D, Sampler3D
	};

	struct MemberUniformBuffer
	{
		std::string Name;
		RadiantShaderType ShaderType = RadiantShaderType::None; 
		RadiantShaderDataType DataType = RadiantShaderDataType::None;
		size_t Size = 0;
		uint32_t Offset = 0;
	};

	struct SamplerUniform
	{
		std::string Name;
		RadiantShaderType ShaderType = RadiantShaderType::None;
		RadiantShaderSamplerDataType DataType = RadiantShaderSamplerDataType::None;
		BindingPoint Binding;
	};

	typedef std::unordered_map<std::string, MemberUniformBuffer> UniformBuffer;

	struct ShaderUniformBuffer
	{
		std::string Name;
		uint32_t Index;
		BindingPoint Binding;
		uint32_t Size;
		uint32_t RenderingID;

		UniformBuffer Uniforms;
	};

	class Shader : public Memory::RefCounted
	{
	public:
		virtual ~Shader() = default;

		virtual void Use(BindUsage use = BindUsage::Bind) const = 0;
		virtual void Reload() = 0;

		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec3& value) const = 0;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, float value) const = 0;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, bool value) const = 0;

		static Memory::Shared<Shader> Create(const std::filesystem::path& path);
	};
}