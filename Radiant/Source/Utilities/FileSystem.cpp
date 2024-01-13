#include <Radiant/Utilities/FileSystem.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace Radiant::Utils
{
	class WindowsFileSystem;
	bool FileSystem::CreateDirectory(const std::filesystem::path& directory)
	{
		return fs::create_directory(directory);
	}

	bool FileSystem::CreateDirectory(const std::string& directory)
	{
		return CreateDirectory(fs::path(directory));
	}

	bool FileSystem::Exists(const std::filesystem::path& filepath)
	{
		return fs::exists(filepath);
	}

	bool FileSystem::Exists(const std::string& filepath)
	{
		return fs::exists(fs::path(filepath));
	}

	std::string FileSystem::GetFileName(const std::filesystem::path& filepath)
	{
		return filepath.filename().string();
	}

	std::string FileSystem::GetFileName(const std::string& filepath)
	{
		return std::filesystem::path(filepath).filename().string();
	}

	std::filesystem::path FileSystem::OpenFileDialog(const char* filter)
	{
		return {};//WindowsFileSystem::OpenFileDialog(filter);
	}

	std::filesystem::path FileSystem::GetFileDirectory(const std::filesystem::path& filepath)
	{
		return filepath.parent_path();
	}

	std::string FileSystem::GetFileDirectoryString(const std::filesystem::path& filepath)
	{
		return filepath.parent_path().string();
	}

	std::string FileSystem::ReadFileContent(const std::filesystem::path& filepath)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		RADIANT_VERIFY(in, "Could not load shader! {}", filepath.string().c_str());

		std::string fileContent;
	
		in.seekg(0, std::ios::end);
		fileContent.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&fileContent[0], fileContent.size());
		in.close();

		return fileContent;
	}

}