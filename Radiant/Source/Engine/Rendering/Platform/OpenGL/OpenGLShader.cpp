#include <glad/glad.h>
#include <shaderc/shaderc.hpp>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fs = std::filesystem;

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

		static fs::path GetBinaryPathByType(RadiantShaderType type, const fs::path& shaderFile)
		{
			switch (type)
			{
				case RadiantShaderType::Vertex:
					return Constants::Path::RESOURCE_SPIRV_BINARY / (shaderFile.stem().string() + Constants::Extensions::SPIRV_BINARY_EXTENSION_VERT);
				case RadiantShaderType::Fragment:
					return Constants::Path::RESOURCE_SPIRV_BINARY / (shaderFile.stem().string() + Constants::Extensions::SPIRV_BINARY_EXTENSION_FRAG);
				case RadiantShaderType::Compute:
					return Constants::Path::RESOURCE_SPIRV_BINARY / (shaderFile.stem().string() + Constants::Extensions::SPIRV_BINARY_EXTENSION_COMP);
			}

			return {};
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

		if (!Utils::FileSystem::Exists(Constants::Path::RESOURCE_SPIRV_BINARY))
		{
			if (!fs::exists(Constants::Path::RESOURCE_SPIRV_BINARY.parent_path())) {
				fs::create_directories(Constants::Path::RESOURCE_SPIRV_BINARY.parent_path());
			}
			Utils::FileSystem::CreateDirectory(Constants::Path::RESOURCE_SPIRV_BINARY);
		}

		CompileToSPIR_V();

		for (const auto& sh : m_ShaderBinary)
		{
			UpdateBinaryFile(m_FilePath, Utils::GetBinaryPathByType(sh.first, m_FilePath), sh.second);
		}

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

		for (const spirv_cross::Resource& resource : res.gl_plain_uniforms)
		{	
			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				uint32_t offset = 0;
				for (uint32_t index = 0; index < type.member_types.size(); ++index)
				{
					uint32_t member_type_id = type.member_types[index]; 
					const spirv_cross::SPIRType& member_type = compiler.get_type(member_type_id);
				
					const std::string& member_name = compiler.get_member_name(resource.base_type_id, index);
					const auto dataType = Utils::SPIRTypeToShaderDataType(member_type);
					const auto size = Shader::GetDataTypeSize(dataType);

					const auto fullname = resource.name + '.' + member_name;
					auto& buffer = m_Uniforms[fullname];

					buffer = { fullname, shadertype, dataType, OGLGetUniformPosition(fullname), size, offset, m_UniformTotalOffset };
					offset += size;
					m_UniformTotalOffset += size;
				}
			}

			else
			{
				const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
				const auto dataType = Utils::SPIRTypeToShaderDataType(type);
				const auto size = Shader::GetDataTypeSize(dataType);

				auto& buffer = m_Uniforms[resource.name];

				buffer = { resource.name, shadertype, dataType, OGLGetUniformPosition(resource.name), size, 0, m_UniformTotalOffset };
				m_UniformTotalOffset += size;
			}
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
			std::string bufferName = resource.name;
			
			if (m_UniformBuffers.find(bindingPoint) == m_UniformBuffers.end())
			{
				ShaderUniformBufferObject& buffer = m_UniformBuffers[bindingPoint];
				buffer.Name = bufferName;
				buffer.Binding = bindingPoint;
				buffer.Size = bufferSize;

				buffer.Uniforms.reserve(memberCount);

				glCreateBuffers(1, &buffer.RenderingID);
				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferData(GL_UNIFORM_BUFFER, buffer.Size, nullptr, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, buffer.Binding, buffer.RenderingID); 
				
				RA_TRACE("Created (ID:{0}) Uniform Buffer at binding point {1} with name '{2}', size is {3} bytes", buffer.RenderingID, buffer.Binding, buffer.Name, buffer.Size);

				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				//glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer.RenderingID, 0, buffer.Size);

				for (int i = 0; i < memberCount; i++)
				{
					const auto type = compiler.get_type(bufferType.member_types[i]);
					const auto name = compiler.get_member_name(bufferType.self, i);
					const auto size = compiler.get_declared_struct_member_size(bufferType, i);
					const auto offset = compiler.type_struct_member_offset(bufferType, i);

					RadiantShaderDataType uniformType = Utils::SPIRTypeToShaderDataType(type);
					buffer.Uniforms[name] = { name, shadertype, uniformType , size, offset };
				}
			}

			else
			{
				auto& buffer = m_UniformBuffers[bindingPoint];

				RADIANT_VERIFY(buffer.Name == resource.name);
				glDeleteBuffers(1, &buffer.RenderingID);
				glCreateBuffers(1, &buffer.RenderingID);

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferData(GL_UNIFORM_BUFFER, buffer.Size, nullptr, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, buffer.Binding, buffer.RenderingID);

				RADIANT_VERIFY("Resized Uniform Buffer at binding point {0} with name '{1}', size is {2} bytes", buffer.Binding, buffer.Name, buffer.Size);
			}

			// ======== Sampler ========
		}
		int32_t sampler = 0;
		for (const spirv_cross::Resource& resource : res.sampled_images)
		{
			auto& type = compiler.get_type(resource.base_type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			const auto& name = resource.name;
			GLint location = OGLGetUniformPosition(name.c_str());
			RADIANT_VERIFY(location != -1);

			glUniform1i(location, binding);

			m_Resources[name] = { name, shadertype, Utils::GetDimensionSampler(type.image.dim), binding };
		}
	}

	void OpenGLShader::ParseConstantBuffers(RadiantShaderType shaderType, const std::vector<uint32_t>& data)
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

	void OpenGLShader::UploadFromBinaryFile(const std::filesystem::path& path, const std::filesystem::path& binaryPath)
	{
		/*if (Utils::FileSystem::Exists(binaryPath))
		{
			auto binaryFileTime = fs::last_write_time(binaryPath);
			auto fileTime = fs::last_write_time(path);
			if (binaryFileTime >= fileTime)
			{
				std::ifstream file(binaryPath);

				if (file.is_open())
				{
					std::vector<uint32_t> binray
					auto fileSize = fs::file_size(binaryPath);
					file.read((const char*)binary.data(), binary.size());
					file.close();
				}
				else
				{
					RADIANT_VERIFY(false);
				}
			}
		}*/
	}

	void OpenGLShader::UpdateBinaryFile(const std::filesystem::path& path, const std::filesystem::path& binaryPath, const std::vector<uint32_t>& binary)
	{
		if (!Utils::FileSystem::Exists(binaryPath))
		{
			Utils::FileSystem::CreateFile(binaryPath);
		}

		auto fileSize = fs::file_size(binaryPath);
		auto binaryFileTime = fs::last_write_time(binaryPath);
		auto fileTime = fs::last_write_time(path);
		if (!fileSize || binaryFileTime < fileTime)
		{
			std::ofstream file(binaryPath);

			if (file.is_open())
			{
				file.write((const char*)binary.data(), binary.size());
				file.close();
			}
			else
			{
				RADIANT_VERIFY(false);
			}
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

		for (auto& shaderData : m_ShaderBinary)
		{
			ParseBuffers(shaderData.first, shaderData.second);
		}
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

	void OpenGLShader::UploadUniformMat4(int32_t location, const glm::mat4& values)
	{
		if (location != -1)
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(values));
		else
			RA_WARN("Uniform 'X' not found!");
	}

	GLuint OpenGLShader::OGLGetUniformPosition(const std::string& name)
	{
		auto pos = glGetUniformLocation(m_RenderingID, name.c_str()); 
		if(pos == -1)
			RA_WARN("{}: could not find uniform location {}", name, pos);
		return pos;
	}

	void OpenGLShader::Use(BindUsage use) const
	{
		Memory::Shared< const OpenGLShader> instance = this;
		Rendering::SubmitCommand([instance, use]() mutable
			{
				if (use == BindUsage::Unbind)
				{
					glUseProgram(0);
					return;
				}
				glUseProgram(instance->m_RenderingID);
			});
	}
}