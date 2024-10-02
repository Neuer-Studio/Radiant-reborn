#pragma once

#include <shaderc/shaderc.hpp>
#include <shaderc/shaderc_util/file_finder.h>

namespace Radiant::Preprocessing
{
	class GLSLIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		explicit GLSLIncluder(const shaderc_util::FileFinder* file_finder);

		virtual shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override;

		virtual void ReleaseInclude(shaderc_include_result* data) override;

		~GLSLIncluder() = default;
	private:
		const shaderc_util::FileFinder m_FileFinder;
	};
}