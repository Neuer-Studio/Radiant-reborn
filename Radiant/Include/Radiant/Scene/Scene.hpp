#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Radiant/Rendering/Mesh.hpp>

#include <entt/entt.hpp>

namespace Radiant
{
	struct Environment;
	struct SceneRendering;
	class Entity;

	struct DirectionalLight
	{
		glm::vec3 Direction;
		alignas(16) glm::vec3 Radiance; // NOTE: GLSL interprets vec3 (12bytes) as vec4 (16bytes)
		
		float Intensity;
		bool CastShadows;
	};

	struct PointLight
	{
		glm::vec3 Direction;
		alignas(16) glm::vec3 Radiance; // NOTE: GLSL interprets vec3 (12bytes) as vec4 (16bytes)

		float Intensity;
		float Radius;
		float Falloff;
		float LightSize;
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights;
		std::vector<PointLight> PointLights;

		[[nodiscard]] uint32_t GetPointLightsSize() const { return (uint32_t)(PointLights.size() * sizeof(PointLight)); }
	};

	struct SceneUpdateInformation
	{
		Timestep TimeStep;
		Camera Camera;
		uint32_t Width;
		uint32_t Height;
	};

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		Entity GetMainCameraEntity();

		void OnUpdate(const SceneUpdateInformation& information);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;
		LightEnvironment GetLightEnvironment() const { return m_LightEnvironment; }

		const auto& GetSceneUpdateInfo() const { return m_Information; }

		inline const uint32_t GetSceneSamplesCount() const { return m_SamplesCount; }

		void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const;
		const Memory::Shared<Image2D>& GetFinalPassImage() const;
		void SetEnvMapRotation(float rotation);
		void SetIBLContribution(float value);

	private:
		uint32_t m_SamplesCount = 2;

		std::string m_SceneName;
		entt::registry m_Registry;

		LightEnvironment m_LightEnvironment;
		SceneUpdateInformation m_Information;

		friend class Entity;
		friend class PanelOutliner;
		friend class SceneRenderingPanel;
	};
}