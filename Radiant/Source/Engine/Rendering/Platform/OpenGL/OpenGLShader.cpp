#include <glad/glad.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/shaderc_util/file_finder.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Rendering/ShaderPreprocessing/GLSLIncluder.hpp>

namespace fs = std::filesystem;

#pragma warning(error : 4834)

namespace Radiant
{
	namespace Utils
	{
		static bool IsArray(const spirv_cross::SPIRType& type)
		{
			return type.array.size() > 0;
		}

		static uint32_t GetArraySize(const spirv_cross::SPIRType& type)
		{
			if (!IsArray(type))
				return 1;
			size_t arraySize = 1;
			for (auto size : type.array) {
				arraySize *= size;
			}
			return arraySize;
		}

		static RadiantShaderDataType GetDimensionSampler(spv::Dim type)
		{
			switch (type)
			{
				case spv::Dim::Dim1D:
					return RadiantShaderDataType::Sampler1D;
				case spv::Dim::Dim2D:
					return RadiantShaderDataType::Sampler2D;
				case spv::Dim::Dim3D:
					return RadiantShaderDataType::Sampler3D;
			}

			return RadiantShaderDataType::None;
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
	
		static RadiantShaderType GetRadiantShaderTypeByBinaryExtension(const std::string& extension)
		{
			if (extension == Constants::Extensions::SPIRV_BINARY_EXTENSION_VERT)
				return RadiantShaderType::Vertex;
			else if (extension == Constants::Extensions::SPIRV_BINARY_EXTENSION_FRAG)
				return RadiantShaderType::Fragment;
			else if (extension == Constants::Extensions::SPIRV_BINARY_EXTENSION_COMP)
				return RadiantShaderType::Compute;

			RADIANT_VERIFY(false);
			return RadiantShaderType::None;
		}

		static fs::path GetBinaryPath()
		{
			return Constants::Path::RESOURCE_SPIRV_BINARY;
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

				case spirv_cross::SPIRType::Struct:     return RadiantShaderDataType::Struct;
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
		CreateDirectoriesIfNotFound();

		std::unordered_map<RadiantShaderType, std::vector<uint32_t>>& binarySPIRV = UploadFromBinaryFile(m_FilePath);
		bool statusCached = !binarySPIRV.empty();

		if (statusCached)
		{
			Rendering::SubmitCommand([this, binarySPIRV]()
				{
					Upload(binarySPIRV);
				});
		}
		else
		{
			std::string content = Utils::FileSystem::ReadFileContent(m_FilePath);
			Load(content);
		}
	}

	void OpenGLShader::CreateDirectoriesIfNotFound()
	{
		if (!Utils::FileSystem::Exists(Constants::Path::RESOURCE_SPIRV_BINARY))
		{
			if (!fs::exists(Constants::Path::RESOURCE_SPIRV_BINARY.parent_path())) {
				fs::create_directories(Constants::Path::RESOURCE_SPIRV_BINARY.parent_path());
			}
			Utils::FileSystem::CreateDirectory(Constants::Path::RESOURCE_SPIRV_BINARY);
		}
	}

	std::vector<uint32_t> OpenGLShader::TryToLoadCachedData(const std::filesystem::path& path, RadiantShaderType type)
	{
		const auto& binaryPath = Utils::GetBinaryPathByType(type, m_FilePath);
		if (!Utils::FileSystem::Exists(binaryPath))
		{
			return {};
		}
		auto fileSize = fs::file_size(binaryPath);
		auto binaryFileTime = fs::last_write_time(binaryPath);
		auto fileTime = fs::last_write_time(path);
		if (fileSize && binaryFileTime > fileTime)
		{
			return Utils::FileSystem::ReadByteFileContent(binaryPath);
		}

		return {};
	}

	const std::vector<RadiantShaderType> OpenGLShader::ListShaderTypesByFileName(const std::string& fileName) const
	{
		std::vector<RadiantShaderType> shaderTypes;
		const auto& directory = Utils::GetBinaryPath();

		for (const auto& file : fs::directory_iterator(directory))
		{
			if (Utils::FileSystem::GetFileNameWithoutExtension(file) == fileName)
			{ 
				const auto& extension = Utils::FileSystem::GetFileExtension(file);
				shaderTypes.push_back(Utils::GetRadiantShaderTypeByBinaryExtension(extension));
			}
		}

		return shaderTypes;
	}

	void OpenGLShader::Load(const std::string& shader—ontent)
	{

		const auto& shaderPreProccess = PreProcess(shader—ontent);
		const auto& binarySPIRV = CompileToSPIR_V(shaderPreProccess);

		for (const auto& sh : binarySPIRV)
		{
			const auto& binaryPath = Utils::GetBinaryPathByType(sh.first, m_FilePath);
			FillBinaryFile(m_FilePath, binaryPath, sh.second);
		}

		Rendering::SubmitCommand([this, binarySPIRV]()
			{
				Upload(binarySPIRV);
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
			const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
			if (type.basetype == spirv_cross::SPIRType::Struct)
			{
				uint32_t offset = 0;
				for (uint32_t index = 0; index < type.member_types.size(); ++index)
				{
					uint32_t member_type_id = type.member_types[index]; 
					const spirv_cross::SPIRType& member_type = compiler.get_type(member_type_id);
				
					const std::string& member_name = compiler.get_member_name(resource.type_id, index);
					const auto dataType = Utils::SPIRTypeToShaderDataType(member_type);
					const auto size = Shader::GetDataTypeSize(dataType);

					const auto fullname = resource.name + '.' + member_name;
					auto& buffer = m_Uniforms[fullname];

					buffer = { { fullname, shadertype, dataType, OGLGetUniformPosition(fullname) }, size, offset, m_UniformTotalOffset };
					offset += size;
					m_UniformTotalOffset += size;
				}
			}

			else
			{
				const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
				std::optional<uint32_t> arraySize;
				if (Utils::IsArray(type))
				{
					arraySize = Utils::GetArraySize(type);
				}
				const auto dataType = Utils::SPIRTypeToShaderDataType(type);
				const auto size = Shader::GetDataTypeSize(dataType);

				auto& buffer = m_Uniforms[resource.name];

				buffer = { { resource.name, shadertype, dataType, OGLGetUniformPosition(resource.name), arraySize }, size, 0, m_UniformTotalOffset };
				
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
			
			if (s_UniformBuffers.find(bindingPoint) == s_UniformBuffers.end())
			{
				ShaderUniformBufferObject& buffer = s_UniformBuffers[bindingPoint];
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
				auto& buffer = s_UniformBuffers[bindingPoint]; // TODDO: static

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

			const auto& type = compiler.get_type(resource.type_id);
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			const auto& name = resource.name;
			GLint location = OGLGetUniformPosition(name.c_str());
			//RADIANT_VERIFY(location != -1);
			std::optional<uint32_t> arraySize;
			if (Utils::IsArray(type))
			{
				arraySize = Utils::GetArraySize(type);
			}

			glUniform1i(location, binding);

			m_Resources[name] = { name, shadertype, Utils::GetDimensionSampler(type.image.dim), binding, arraySize };
		}
	}

	void OpenGLShader::ParseConstantBuffers(RadiantShaderType shaderType, const std::vector<uint32_t>& data)
	{
		
	}

	const std::unordered_map<RadiantShaderType, std::vector<uint32_t>> OpenGLShader::CompileToSPIR_V(const std::unordered_map<RadiantShaderType, std::string>& shaderSource)
	{
		std::unordered_map<RadiantShaderType, std::vector<uint32_t>> output;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		shaderc_util::FileFinder fileFinder;
		options.SetIncluder(std::make_unique<Preprocessing::GLSLIncluder>(&fileFinder));

		for (const auto source : shaderSource)
		{
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source.second, Utils::ShaderTypeToShader_C(source.first), m_FilePath.string().c_str(), options);
			if (module.GetCompilationStatus() != shaderc_compilation_status_success) 
			{
				RA_ERROR("{}: {}", Utils::RadiantShaderToString(source.first), module.GetErrorMessage());
				RADIANT_VERIFY(false);
			}
			output[source.first] = { module.cbegin(), module.cend() };
		}

		return output;
	}

	std::unordered_map<RadiantShaderType, std::vector<uint32_t>> OpenGLShader::UploadFromBinaryFile(const std::filesystem::path& path)
	{
		std::unordered_map<RadiantShaderType, std::vector<uint32_t>> outputBinary;
		std::string filename = Utils::FileSystem::GetFileNameWithoutExtension(path);
		const auto& types = ListShaderTypesByFileName(filename);
		for (const auto& shader : types)
		{
			outputBinary[shader] = std::move(TryToLoadCachedData(m_FilePath, shader));
			if (outputBinary[shader].empty())
			{
				return {};
			}
		}

		return outputBinary;
	}

	void OpenGLShader::FillBinaryFile(const std::filesystem::path& path, const std::filesystem::path& binaryPath, const std::vector<uint32_t>& binary)
	{
		std::ofstream file(binaryPath, std::ios::trunc | std::ios::binary);

		if (file.is_open())
		{
			file.write(reinterpret_cast<const char*>(binary.data()), binary.size() * sizeof(uint32_t));
			file.close();
		}
		else
		{
			RADIANT_VERIFY(false);
		}
	}

	void OpenGLShader::Upload(const std::unordered_map<RadiantShaderType, std::vector<uint32_t>>& shaderBinary)
	{
		if (m_RenderingID)
			glDeleteProgram(m_RenderingID);

		std::vector<GLuint> shaderRendererIDs;
		m_RenderingID = glCreateProgram();

		for (const auto& kv : shaderBinary)
		{
			GLenum type = Utils::OGLShaderTypeFromRadiant(kv.first);
			const std::vector<uint32_t>& binary = kv.second;

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

		for (auto& shaderData : shaderBinary)
		{
			ParseBuffers(shaderData.first, shaderData.second);
		}
	}

	const std::unordered_map<RadiantShaderType, std::string> OpenGLShader::PreProcess(const std::string& content)
	{
		std::unordered_map<RadiantShaderType, std::string> output;

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

			output[shaderType] = content.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? content.size() - 1 : nextLinePos));
		}

		return output;
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