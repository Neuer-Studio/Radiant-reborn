#pragma once

#include <filesystem>

namespace Radiant::Constants
{
	namespace Path
	{
		const std::filesystem::path RESOURCE_PATH = "Resources/";
		const std::filesystem::path RESOURCE_SPIRV_BINARY = "Resources/Shaders/SPIRV/Bin/";
	}

	namespace Extensions
	{
		const std::string SPIRV_BINARY_EXTENSION_VERT = ".spvbin_vert";
		const std::string SPIRV_BINARY_EXTENSION_FRAG = ".spvbin_frag";

	}
}