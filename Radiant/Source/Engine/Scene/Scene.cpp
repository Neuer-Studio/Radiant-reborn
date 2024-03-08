#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/Entity.hpp>

#include <Radiant/Scene/SceneRendering.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{
	static SceneRendering* s_SceneRendering = nullptr;

	Scene::Scene(const std::string& sceneName)
		: m_SceneName(sceneName)
	{
		if (s_SceneRendering == nullptr)
			s_SceneRendering = SceneRendering::Create(this);
	}

	Scene::~Scene()
	{
		delete s_SceneRendering;
	}

	Radiant::Entity Scene::CreateEntity(const std::string& name /*= ""*/)
	{
		auto entity = Entity{m_Registry.create(), this};
		auto& idComponent = entity.AddComponent<IDComponent>();
		idComponent.ID = {};
		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		return entity;
	}

	Entity Scene::GetMainCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (const auto& entity : view)
		{
			auto& comp = view.get<CameraComponent>(entity);
			if (comp.Primary)
				return { entity, this };
		}
		return {};
	}

	void Scene::OnUpdate(Timestep ts, Camera& camera)
	{
	/*	Entity& cameraEntity = GetMainCameraEntity();
		if (!cameraEntity)
			return;*/
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);

		auto lights = m_Registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
		uint32_t directionalLightIndex = 0;
		for (auto entity : lights)
		{
			auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
			glm::vec3 direction = -glm::normalize(glm::mat3(transformComponent.GetTransform()) * glm::vec3(1.0f));
			m_LightEnvironment.DirectionalLights =
			{
				direction,
				lightComponent.Radiance,
			};
		}


		for (auto entity : group)
		{
			auto [transformComponent, meshComponent] = group.get<TransformComponent, MeshComponent>(entity);
			if (meshComponent.Mesh)
			{
				s_SceneRendering->SubmitMesh(meshComponent, transformComponent.GetTransform());
			}
		}

		s_SceneRendering->BeginScene(camera);
		s_SceneRendering->OnUpdate(ts);

	}

	void Scene::SetEnvironment(const Environment& env)
	{
		s_SceneRendering->SetEnvironment(env);
	}

	Environment Scene::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		return s_SceneRendering->CreateEnvironmentScene(filepath);
	}

	void Scene::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const
	{
		s_SceneRendering->SubmitMesh(mesh, transform);
	}

	SceneRendering* Scene::GetSceneRendering()
	{
		return s_SceneRendering;
	}

}