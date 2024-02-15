#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Radiant/Rendering/Mesh.hpp>

namespace Radiant
{
	struct Environment;
	struct SceneRendering;

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);

		void OnUpdate(Timestep ts, Camera* cam);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;

		void AddMesh(const Memory::Shared<Mesh>& mesh) const;

		static Memory::Shared<SceneRendering>& GetSceneRendering();
	private:
		std::string m_SceneName;
	};
}