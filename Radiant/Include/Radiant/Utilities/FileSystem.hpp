#pragma once

//NOTE: This is a workaround for Microsoft macros so that 
//we can use names like CreateDirectory, etc

#ifdef CreateDirectory
#undef CreateDirectory
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef CreateFile
#undef SetEnvironmentVariable
#undef GetEnvironmentVariable
#endif

#include <functional>
#include <filesystem>

namespace Radiant::Utils
{
	class FileSystem
	{
	public:
		static std::string GetFileName(const std::filesystem::path& filepath);
		static std::string GetFileName(const std::string& filepath);
		static std::string ReadFileContent(const std::filesystem::path& filepath);
		static bool CreateDirectory(const std::filesystem::path& directory);
		static bool CreateDirectory(const std::string& directory);
		static void CreateFile(const std::string& path);
		static void CreateFile(const std::filesystem::path& path);
		static bool Exists(const std::filesystem::path& filepath);
		static bool Exists(const std::string& filepath);
		static std::string GetFileDirectoryString(const std::filesystem::path& filepath);
		static std::filesystem::path GetFileDirectory(const std::filesystem::path& filepath);
	public:
		static std::filesystem::path OpenFileDialog(const char* filter = "All\0*.*\0");
		static std::filesystem::path OpenFolderDialog(const char* initialFolder = "");
		static std::filesystem::path SaveFileDialog(const char* filter = "All\0*.*\0");
	public:
		static bool HasEnvironmentVariable(const std::string& key);
		static bool SetEnvironmentVariable(const std::string& key, const std::string& value);
		static std::string GetEnvironmentVariable(const std::string& key);
	public:
		std::string ReadFileAndSkipBOM(const std::filesystem::path& filepath);
	};
}