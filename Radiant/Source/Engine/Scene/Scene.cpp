#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/Entity.hpp>

#include <Radiant/Scene/SceneRendering.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{

	Scene::Scene(const std::string& sceneName)
		: m_SceneName(sceneName)
	{
	
	}

	Scene::~Scene()
	{
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

	void Scene::OnUpdate(const SceneUpdateInformation& information)
	{
	/*	Entity& cameraEntity = GetMainCameraEntity();
		if (!cameraEntity)
			return;*/

		auto dirLight = m_Registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
		for (auto entity : dirLight)
		{
			auto [transformComponent, lightComponent] = dirLight.get<TransformComponent, DirectionalLightComponent>(entity);
			glm::vec3 direction = -glm::normalize(glm::mat3(transformComponent.GetTransform()) * glm::vec3(1.0f));
			m_LightEnvironment.DirectionalLights =
			{
				direction,
				lightComponent.Radiance,
				lightComponent.Intensity,
				lightComponent.CastShadows
			};
		}

		// Point lights
		{
			auto pointLights = m_Registry.group<PointLightComponent>(entt::get<TransformComponent>);
			uint32_t pointLightIndex = 0;
			for (auto entity : pointLights)
			{
				auto [transformComponent, lightComponent] = pointLights.get<TransformComponent, PointLightComponent>(entity);
				glm::vec3 direction = transformComponent.Translation; //TODO: flag paneloutliner only translation
				m_LightEnvironment.PointLights.resize(pointLights.size());
				m_LightEnvironment.PointLights[pointLightIndex++] =
				{
					direction,
					lightComponent.Radiance,
					lightComponent.Intensity,
					lightComponent.Radius,
					lightComponent.Falloff,
					lightComponent.LightSize,
				};
			}
		}

		auto envMap = m_Registry.group<EnvironmentMap>(entt::get<TransformComponent>);
		for (auto entity : envMap)
		{
			auto [transformComponent, envMapComponent] = envMap.get<TransformComponent, EnvironmentMap>(entity);
			EnvironmentAttributes attrs;
			attrs.EnvironmentMapLod = envMapComponent.EnvironmentMapLod;
			attrs.Intensity = envMapComponent.Intensity;

			SceneRendering::Get().SetEnvironmentAttributes(attrs);
		}

		auto mesh = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		for (auto entity : mesh)
		{
			auto [transformComponent, meshComponent] = mesh.get<TransformComponent, MeshComponent>(entity);
			if (meshComponent.Mesh)
			{
				SceneRendering::Get().SubmitMesh(meshComponent, transformComponent.GetTransform());
			}
		}

		SceneRendering::Get().BeginScene(this, information.Camera);
		SceneRendering::Get().OnUpdate(information.TimeStep);
		SceneRendering::Get().SetSceneVeiwPortSize({ information.Width, information.Height });
		SceneRendering::Get().EndScene();

	}

	void Scene::SetEnvironment(const Environment& env)
	{
		SceneRendering::Get().SetEnvironment(env);
	}

	Environment Scene::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		return SceneRendering::Get().CreateEnvironmentMap(filepath);
	}

	void Scene::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const
	{
		SceneRendering::Get().SubmitMesh(mesh, transform);
	}

	const Radiant::Memory::Shared<Radiant::Image2D>& Scene::GetFinalPassImage() const
	{
		return SceneRendering::Get().GetFinalPassImage();
	}

	void Scene::SetEnvMapRotation(float rotation)
	{
		SceneRendering::Get().SetEnvMapRotation(rotation);
	}

	void Scene::SetIBLContribution(float value)
	{
		SceneRendering::Get().SetIBLContribution(value);
	}

}