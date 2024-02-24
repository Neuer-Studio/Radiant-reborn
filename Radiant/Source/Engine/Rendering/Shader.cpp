#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Shader.hpp>

namespace Radiant
{

	const uint32_t Shader::GetDataTypeSize(RadiantShaderDataType dataType)
	{
		switch (dataType)
		{
		case RadiantShaderDataType::Bool:
			return 4; // bool is 4 bytes

		case RadiantShaderDataType::Float:
		case RadiantShaderDataType::Int:
		case RadiantShaderDataType::UInt:
			return 4;

		case RadiantShaderDataType::Float2:
		case RadiantShaderDataType::Int2:
			return 4 * 2;

		case RadiantShaderDataType::Float3:
		case RadiantShaderDataType::Int3 :
			return 4 * 3;

		case RadiantShaderDataType::Float4:
		case RadiantShaderDataType::Int4:
			return 4 * 4;

		case RadiantShaderDataType::Mat3:
			return sizeof(glm::mat3);
		case RadiantShaderDataType::Mat4:
			return sizeof(glm::mat4);
		}
	}

	Radiant::Memory::Shared<Radiant::Shader> Shader::Create(const std::filesystem::path& path)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				return Memory::Shared<OpenGLShader>::Create(path);
			}
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

	void ShaderLibrary::Add(const Memory::Shared<Shader>& shader)
	{
		auto& name = shader->GetShaderName();
		RADIANT_VERIFY(m_Shaders.find(name) == m_Shaders.end(), "");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Load(const std::filesystem::path& filepath)
	{
		auto shader = Memory::Shared<Shader>(Shader::Create(filepath));
		auto& name = shader->GetShaderName();
		RADIANT_VERIFY(m_Shaders.find(name) == m_Shaders.end(), "");
		m_Shaders[name] = shader;
	}
	void ShaderLibrary::Load(const std::string& name, const std::filesystem::path& filepath)
	{
		RADIANT_VERIFY(m_Shaders.find(name) == m_Shaders.end(), "");
		m_Shaders[name] = Memory::Shared<Shader>(Shader::Create(filepath));
	}

	const Memory::Shared<Shader>& ShaderLibrary::Get(const std::string& name) const
	{
		RADIANT_VERIFY(m_Shaders.find(name) != m_Shaders.end(), "");
		return m_Shaders.at(name);
	}

}