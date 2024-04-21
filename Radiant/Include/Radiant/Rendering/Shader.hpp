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
		Bool,

		Sampler1D, Sampler2D, Sampler3D,

		Struct
	};

	struct MemberUniformBufferObject
	{
		std::string Name;
		RadiantShaderType ShaderType = RadiantShaderType::None; 
		RadiantShaderDataType DataType = RadiantShaderDataType::None;
		size_t Size = 0;
		uint32_t Offset = 0;
	};

	struct UniformBase
	{
		std::string Name;
		RadiantShaderType ShaderType = RadiantShaderType::None;
		RadiantShaderDataType DataType = RadiantShaderDataType::None;
		BindingPoint Binding;
	};

	struct Uniform
	{
		UniformBase Uniform;
		uint32_t Size = 0;
		uint32_t offset = 0; // NOTE: using in blocks (between fields) 
		uint32_t totalOffset = 0;// NOTE: total offset (between all uniforms) (using for materials to save data in heap)
	};

	struct SamplerUniform
	{
		UniformBase Uniform;
	};

	typedef std::unordered_map<std::string, MemberUniformBufferObject> UBO;

	struct ShaderUniformBufferObject
	{
		std::string Name;
		uint32_t Index;
		BindingPoint Binding;
		uint32_t Size;
		uint32_t RenderingID;

		UBO Uniforms;
	};

	class Shader : public Memory::RefCounted
	{
	public:
		virtual ~Shader() = default;

		virtual void Use(BindUsage use = BindUsage::Bind) const = 0;
		virtual void Reload() = 0;
		virtual const std::string GetShaderName() const = 0;
		virtual RenderingID GetRenderingID() const = 0;

		static const uint32_t GetDataTypeSize(RadiantShaderDataType dataType);

		static Memory::Shared<Shader> Create(const std::filesystem::path& path);

		// Temporary, before we have an asset manager
		static inline std::vector<Memory::Shared<Shader>> s_AllShaders;
	};

	class ShaderLibrary : public Memory::RefCounted
	{
	public:
		void Add(const Memory::Shared<Shader>& shader);
		void Load(const std::string& name, const std::filesystem::path& filepath);
		void Load(const std::filesystem::path& filepath);

		const Memory::Shared<Shader>& Get(const std::string& name) const;
		std::unordered_map<std::string, Memory::Shared<Shader>>& GetShaders() { return m_Shaders; }
	private:
		std::unordered_map<std::string, Memory::Shared<Shader>> m_Shaders;
	};
}