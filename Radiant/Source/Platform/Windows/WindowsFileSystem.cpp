#include "WindowsFileSystem.hpp"
#include <Radiant/Core/Application.hpp>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <filesystem>
#include <commdlg.h>

namespace Radiant::Utils
{
	std::filesystem::path WindowsFileSystem::OpenFileDialog(const char* filter)
	{

		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetInstance().GetWindow()->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			std::string fp = ofn.lpstrFile;
			std::replace(fp.begin(), fp.end(), '\\', '/');
			return std::filesystem::path(fp);
		}

		return std::filesystem::path();
	}
}