#include <Radiant/Rendering/Scene/Scene.hpp>

#include <Radiant/Rendering/Scene/SceneRendering.hpp>

namespace Radiant
{
	static Memory::Shared<SceneRendering> s_SceneRendering = nullptr;

	Scene::Scene(const std::string& sceneName)
		: m_SceneName(sceneName)
	{
		if (s_SceneRendering.Ptr() == nullptr)
			s_SceneRendering = SceneRendering::Create(this);
	}

	void Scene::UpdateScene(Camera* cam)
	{
		s_SceneRendering->SubmitScene(cam);
	}

	void Scene::SetEnvironment(const Environment& env)
	{
		s_SceneRendering->SetEnvironment(env);
	}

	Environment Scene::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		return s_SceneRendering->CreateEnvironmentScene(filepath);
	}

	Radiant::Memory::Shared<Radiant::SceneRendering>& Scene::GetSceneRendering()
	{
		return s_SceneRendering;
	}

}