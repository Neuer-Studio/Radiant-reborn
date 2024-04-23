#include <Radiant/Rendering/ShaderPreprocessing/GLSLIncluder.hpp>

#include <Radiant/Utilities/FileSystem.hpp>

namespace Radiant::Preprocessing
{
	
	GLSLIncluder::GLSLIncluder(const shaderc_util::FileFinder* file_finder)
		: m_FileFinder(*file_finder)
	{

	}

	shaderc_include_result* GLSLIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
	{
		const std::filesystem::path full_path =
			(type == shaderc_include_type_relative) ? m_FileFinder.FindRelativeReadableFilepath(requesting_source, requested_source) : m_FileFinder.FindReadableFilepath(requested_source);

		if (full_path.empty())
			RADIANT_VERIFY("Cannot find or open include file {}", full_path);

		const std::string name = std::string(requested_source);
		const std::string contents = Utils::FileSystem::ReadFileContent(full_path);

		auto* const container = new std::array<std::string, 2>;
		(*container)[0] = name;
		(*container)[1] = contents;

		auto const data = new shaderc_include_result;

		data->user_data = container;

		data->source_name = (*container)[0].data();
		data->source_name_length = (*container)[0].size();

		data->content = (*container)[1].data();
		data->content_length = (*container)[1].size();

		return data;
	}

	void GLSLIncluder::ReleaseInclude(shaderc_include_result* data)
	{
		delete static_cast<std::array<std::string, 2>*>(data->user_data);
		delete data;
	}

}