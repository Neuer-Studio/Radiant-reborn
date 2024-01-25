#include <glad/glad.h>
#include <shaderc/shaderc.hpp>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static RadiantShaderSamplerDataType GetDimensionSampler(spv::Dim type)
		{
			switch (type)
			{
				case spv::Dim::Dim1D:
					return RadiantShaderSamplerDataType::Sampler1D;
				case spv::Dim::Dim2D:
					return RadiantShaderSamplerDataType::Sampler2D;
				case spv::Dim::Dim3D:
					return RadiantShaderSamplerDataType::Sampler3D;
			}

			return RadiantShaderSamplerDataType::None;
		}

		static RadiantShaderDataType SPIRTypeToShaderDataType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
				case spirv_cross::SPIRType::Boolean:  return RadiantShaderDataType::Bool;
				case spirv_cross::SPIRType::Int:     
					if (type.vecsize == 1)            return RadiantShaderDataType::Int;
					if (type.vecsize == 2)            return RadiantShaderDataType::Int2;
					if (type.vecsize == 3)            return RadiantShaderDataType::Int3;
					if (type.vecsize == 4)            return RadiantShaderDataType::Int4;

				case spirv_cross::SPIRType::UInt:     return RadiantShaderDataType::UInt;
				case spirv_cross::SPIRType::Float:
					if (type.columns == 3)            return RadiantShaderDataType::Mat3;
					if (type.columns == 4)            return RadiantShaderDataType::Mat4;

					if (type.vecsize == 1)            return RadiantShaderDataType::Float;
					if (type.vecsize == 2)            return RadiantShaderDataType::Float2;
					if (type.vecsize == 3)            return RadiantShaderDataType::Float3;
					if (type.vecsize == 4)            return RadiantShaderDataType::Float4;
					break;
			}
			RADIANT_VERIFY(false, "Unknown type!");
			return RadiantShaderDataType::None;
		}

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

		static std::string RadiantShaderToString(RadiantShaderType type)
		{
			switch (type)
			{
				case Radiant::RadiantShaderType::Vertex:
					return "Vertex";
				case Radiant::RadiantShaderType::Fragment:
					return "Fragment";
				case Radiant::RadiantShaderType::Compute:
					return "Compute";
			}
			return "NONE";
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
		Rendering::SubmitCommand([this]()
			{
				Upload();
			});
	
	}

	void OpenGLShader::ParseBuffers(RadiantShaderType shadertype, const std::vector<uint32_t>& data)
	{
		spirv_cross::Compiler compiler(data);
		spirv_cross::ShaderResources res = compiler.get_shader_resources();

		RA_TRACE("OpenGLShader::ParseBuffers - {0} {1}", m_FilePath.string(), Utils::RadiantShaderToString(shadertype));
		RA_TRACE("   {0} Uniform Buffers", res.uniform_buffers.size());
		RA_TRACE("   {0} Resources", res.sampled_images.size());
		RA_TRACE("   {0} Constant Buffers", res.push_constant_buffers.size());

		for (const spirv_cross::Resource& resource : res.push_constant_buffers)
		{
			RADIANT_VERIFY(false);
			const auto& bufferName = resource.name;
		}

		glUseProgram(m_RenderingID);

		// ======= Uniform buffer =======

		uint32_t bufferIndex = 0;
		for (const spirv_cross::Resource& resource : res.uniform_buffers)
		{
			auto& bufferType = compiler.get_type(resource.base_type_id);
			int memberCount = bufferType.member_types.size();
			uint32_t bindingPoint = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);


			if (s_UniformBuffers[shadertype].find(bindingPoint) == s_UniformBuffers[shadertype].end())
			{
				ShaderUniformBuffer& buffer = s_UniformBuffers[shadertype][bindingPoint];
				buffer.Name = resource.name;
				buffer.Binding = bindingPoint;
				buffer.Size = bufferSize;

				buffer.Uniforms.reserve(memberCount);

				glCreateBuffers(1, &buffer.RenderingID);
				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferData(GL_UNIFORM_BUFFER, buffer.Size, nullptr, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, buffer.Binding, buffer.RenderingID);

				for (int i = 0; i < memberCount; i++)
				{
					auto type = compiler.get_type(bufferType.member_types[i]);
					auto name = compiler.get_member_name(bufferType.self, i);
					auto size = compiler.get_declared_struct_member_size(bufferType, i);
					auto offset = compiler.type_struct_member_offset(bufferType, i);

					RadiantShaderDataType uniformType = Utils::SPIRTypeToShaderDataType(type);
					buffer.Uniforms[name] = {name, shadertype, uniformType , size, offset};
				}
			}

			// ======== Sampler ========

			int32_t sampler = 0;
  			for (const spirv_cross::Resource& resource : res.sampled_images)
			{
				auto& type = compiler.get_type(resource.base_type_id);
				auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				const auto& name = resource.name;
				GLint location = glGetUniformLocation(m_RenderingID, name.c_str());
				RADIANT_VERIFY(location != -1);

				glUniform1i(location, binding);

				m_Resources[name] = { name, shadertype, Utils::GetDimensionSampler(type.image.dim), binding};
			}

		}
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

		for(auto& shaderData : m_ShaderBinary)
			ParseBuffers(shaderData.first, shaderData.second);
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

	GLuint OpenGLShader::OGLGetUniformPosition(const std::string& name)
	{
		auto pos = glGetUniformLocation(m_RenderingID, name.c_str());
		if(pos == -1)
			RA_WARN("{0}: could not find uniform location {0}", name);
		return pos;
	}

	void OpenGLShader::Use(BindUsage use) const
	{
		auto id = m_RenderingID;
		Rendering::SubmitCommand([id , use]()
			{
				if (use == BindUsage::Unbind)
				{
					glUseProgram(0);
					return;
				}
				glUseProgram(id);
			});
	}

	void OpenGLShader::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec3& value) const
	{
		Memory::Shared<const OpenGLShader> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->s_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLShader::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec2& value) const
	{
		Memory::Shared<const OpenGLShader> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->s_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLShader::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, float value) const
	{
		Memory::Shared<const OpenGLShader> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->s_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLShader::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, bool value) const
	{
		Memory::Shared<const OpenGLShader> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->s_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				 // glNamedBufferSubData
			});
	}

}