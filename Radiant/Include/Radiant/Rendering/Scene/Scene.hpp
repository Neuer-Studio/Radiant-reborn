#pragma once

#include <Radiant/Core/Camera.hpp>

namespace Radiant
{
	struct Environment;

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);

		void UpdateScene(Camera* cam);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;
	private:
		std::string m_SceneName;
	};
}