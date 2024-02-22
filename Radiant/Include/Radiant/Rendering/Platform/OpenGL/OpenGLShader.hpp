#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Rendering/RenderingTypes.hpp>

#include <spirv_cross/spirv_glsl.hpp>

namespace Radiant
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::filesystem::path& path);
		virtual ~OpenGLShader() override = default;

		virtual void Reload() override;
		virtual void Use(BindUsage use = BindUsage::Bind) const override;

		virtual RenderingID GetRenderingID() const { return m_RenderingID; }

		virtual const std::string GetShaderName() const { return m_Name; }
	private:
	private:
		void Load(const std::string& shader—ontent);
		void ParseBuffers(RadiantShaderType type, const std::vector<uint32_t>& data);
		void ParseConstantBuffers(RadiantShaderType type, const std::vector<uint32_t>& data);
		void CompileToSPIR_V();
		void UploadFromBinaryFile(const std::filesystem::path& path, const std::filesystem::path& binaryPath);
		void Upload();
		void UpdateBinaryFile(const std::filesystem::path& path, const std::filesystem::path& binaryPath, const std::vector<uint32_t>& binary);
		void PreProcess(const std::string& content);
	private:
		GLuint OGLGetUniformPosition(const std::string& name);
	private:
		std::string m_Name;
		std::filesystem::path m_FilePath;
		RenderingID m_RenderingID = 0;

		std::unordered_map<RadiantShaderType, std::string> m_ShaderSource;
		std::unordered_map<RadiantShaderType, std::vector<uint32_t>> m_ShaderBinary;
		std::unordered_map<std::string, SamplerUniform> m_Resources;

		std::unordered_map<BindingPoint, ShaderUniformBuffer> m_UniformBuffers;

	private:
		friend class OpenGLMaterial;
	};
}