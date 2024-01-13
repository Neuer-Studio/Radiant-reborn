#include <glad/glad.h>
#include <shaderc/shaderc.hpp>

#include <spirv_cross/spirv_glsl.hpp>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static RadiantShaderType ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return RadiantShaderType::Vertex;
			if (type == "fragment" || type == "pixel")
				return RadiantShaderType::Fragment;
			if (type == "compute")
				return RadiantShaderType::Compute;

			return RadiantShaderType::None;
		}

		static GLenum OGLShaderTypeFromRadiant(RadiantShaderType type)
		{
			switch (type)
				{
				case Radiant::RadiantShaderType::Vertex:
					return GL_VERTEX_SHADER;
				case Radiant::RadiantShaderType::Fragment:
					return GL_FRAGMENT_SHADER;
				case Radiant::RadiantShaderType::Compute:
					return GL_COMPUTE_SHADER;
			}

			return GL_NONE;
		}

		static shaderc_shader_kind ShaderTypeToShader_C(RadiantShaderType type)
		{
			switch (type)
			{
			case Radiant::RadiantShaderType::Vertex:
				return shaderc_vertex_shader;
			case Radiant::RadiantShaderType::Fragment:
				return shaderc_fragment_shader;
			case Radiant::RadiantShaderType::Compute:
				return shaderc_compute_shader;
			}
			return (shaderc_shader_kind)0;
		}
	}

	OpenGLShader::OpenGLShader(const std::filesystem::path& path)
		:  m_FilePath(path)
	{
		RADIANT_VERIFY(Utils::FileSystem::Exists(m_FilePath));
		m_Name = Utils::FileSystem::GetFileName(m_FilePath);
		Reload();
	}

	void OpenGLShader::Reload()
	{
		std::string content = Utils::FileSystem::ReadFileContent(m_FilePath);
		Load(content);
	}

	void OpenGLShader::Load(const std::string& shader—ontent)
	{
		PreProcess(shader—ontent);
		CompileToSPIR_V();
		Upload();
	
	}

	void OpenGLShader::Parse()
	{
	}

	void OpenGLShader::CompileToSPIR_V()
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		for (const auto source : m_ShaderSource)
		{
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source.second, Utils::ShaderTypeToShader_C(source.first), m_FilePath.string().c_str(), options);
			if (module.GetCompilationStatus() != shaderc_compilation_status_success) 
			{
				RA_ERROR("{}", module.GetErrorMessage());
				RADIANT_VERIFY(false);
			}
			m_ShaderBinary[source.first] = { module.cbegin(), module.cend() };
		}

	}

	void OpenGLShader::Upload()
	{
		if (m_RenderingID)
			glDeleteProgram(m_RenderingID);

		std::vector<GLuint> shaderRendererIDs;
		m_RenderingID = glCreateProgram();

		for (auto& kv : m_ShaderBinary)
		{
			GLenum type = Utils::OGLShaderTypeFromRadiant(kv.first);
			std::vector<uint32_t>& binary = kv.second;

			GLuint shaderRendererID = glCreateShader(type);
			glShaderBinary(1, &shaderRendererID, GL_SHADER_BINARY_FORMAT_SPIR_V, binary.data(), binary.size() * sizeof(uint32_t));
			glSpecializeShader(shaderRendererID, "main", 0, nullptr, nullptr);
			glAttachShader(m_RenderingID, shaderRendererID);

			shaderRendererIDs.emplace_back(shaderRendererID);
		}

		// Link shader program
		glLinkProgram(m_RenderingID);
		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_RenderingID, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RenderingID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RenderingID, maxLength, &maxLength, &infoLog[0]);
			RA_ERROR("Shader compilation failed ({0}):\n{1}", m_FilePath.string(), &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_RenderingID);
			// Don't leak shaders either.
			for (auto id : shaderRendererIDs)
				glDeleteShader(id);
		}

		// Always detach shaders after a successful link.
		for (auto id : shaderRendererIDs)
			glDetachShader(m_RenderingID, id);
	}

	void OpenGLShader::PreProcess(const std::string& content)
	{
		const char* tokenType = "#type";
		const std::size_t sizeTokenType = strlen(tokenType);
		std::size_t pos = content.find(tokenType, 0);

		while (pos != std::string::npos)
		{
			std::size_t eol = content.find_first_of("\r\n", pos);
			RADIANT_VERIFY(eol != std::string::npos, "Syntax error");
			std::size_t begin = pos + sizeTokenType + 1;
			std::string type = content.substr(begin, eol - begin);
			RADIANT_VERIFY(type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invalid shader type specified");

			std::size_t nextLinePos = content.find_first_not_of("\r\n", eol);
			pos = content.find(tokenType, nextLinePos);
			auto shaderType = Utils::ShaderTypeFromString(type);

			m_ShaderSource[shaderType] = content.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? content.size() - 1 : nextLinePos));
		}
	}

	void OpenGLShader::Use() const
	{
		auto id = m_RenderingID;
		if (id == (uint32_t)-1)
			return;
		Rendering::Submit([id]()
			{
				glUseProgram(1);
			});
	}
}