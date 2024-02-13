#pragma once

#include <Radiant/Core/Camera.hpp>

namespace Radiant
{
	struct Environment;
	struct SceneRendering;

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);

		void UpdateScene(Camera* cam);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;

		static Memory::Shared<SceneRendering>& GetSceneRendering();
	private:
		std::string m_SceneName;
	};
}