#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Rendering/RenderingTypes.hpp>

namespace Radiant
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::filesystem::path& path);
		virtual ~OpenGLShader() override = default;

		virtual void Reload() override;
		virtual void Use() const override;

	private:
		void Load(const std::string& shader—ontent);
		void Parse();
		void CompileToSPIR_V();
		void Upload();
		void PreProcess(const std::string& content);
	private:
		std::string m_Name;
		std::filesystem::path m_FilePath;
		RenderingID m_RenderingID = 0;

		std::unordered_map<RadiantShaderType, std::string> m_ShaderSource;
		std::unordered_map<RadiantShaderType, std::vector<uint32_t>> m_ShaderBinary;
	};
}