#pragma once

#include <Radiant/Utilities/FileSystem.hpp>

namespace Radiant::Utils
{
	class WindowsFileSystem
	{
	public:
		static std::filesystem::path OpenFileDialog(const char* filter);
	};
}