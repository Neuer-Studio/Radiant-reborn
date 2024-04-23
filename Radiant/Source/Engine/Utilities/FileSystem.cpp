#include <Radiant/Utilities/FileSystem.hpp>

#if (RADIANT_PLATFORM_WINDOWS)
#include <Platform/Windows/WindowsFileSystem.hpp>
#endif

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

	void FileSystem::CreateFile(const std::string& path)
	{
		CreateFile(fs::path(path));
	}

	void FileSystem::CreateFile(const std::filesystem::path& path)
	{
		std::ofstream file(path);

		if (file.is_open())
		{
			RA_INFO("Created File {}", path.string());
			file.close();
		}

		else
		{
			RADIANT_VERIFY(false);
		}
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
		return WindowsFileSystem::OpenFileDialog(filter);
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
		RADIANT_VERIFY(in, "Could not read file! {}", filepath.string().c_str());

		std::string fileContent;
	
		in.seekg(0, std::ios::end);
		fileContent.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&fileContent[0], fileContent.size());
		in.close();

		return fileContent;
	}
	
	int SkipBOM(std::istream& in)
	{
		char test[4] = { 0 };
		in.seekg(0, std::ios::beg);
		in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			in.seekg(3, std::ios::beg);
			return 3;
		}
		in.seekg(0, std::ios::beg);
		return 0;
	}

	// Returns an empty string when failing.
	std::string FileSystem::ReadFileAndSkipBOM(const std::filesystem::path& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto fileSize = in.tellg();
			const int skippedChars = SkipBOM(in);

			fileSize -= skippedChars - 1;
			result.resize(fileSize);
			in.read(result.data() + 1, fileSize);
			// Add a dummy tab to beginning of file.
			result[0] = '\t';
		}
		in.close();
		return result;
	}
}