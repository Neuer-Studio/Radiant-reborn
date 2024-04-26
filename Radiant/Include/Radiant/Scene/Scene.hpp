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
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		alignas(16) glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f }; // NOTE: GLSL interprets vec3 (12bytes) as vec4 (16bytes)
		
		float Multiplier = 0.0f;
		bool CastShadows;
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights;// [4] ;
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