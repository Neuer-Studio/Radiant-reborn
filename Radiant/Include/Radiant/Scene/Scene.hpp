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
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights;// [4] ;
	};

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		Entity GetMainCameraEntity();

		void OnUpdate(Timestep ts, Camera& camera);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;
		LightEnvironment GetLightEnvironment() const { return m_LightEnvironment; }

		inline const float GetSceneExposure() const { return m_Exposure; }
		inline const uint32_t GetSceneSamplesCount() const { return m_SamplesCount; }

		inline void SetViewportSize(uint32_t width, uint32_t height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const;

		static SceneRendering* GetSceneRendering();
	private:
		float m_Exposure = 0.8f;
		uint32_t m_SamplesCount = 2;

		std::string m_SceneName;
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		LightEnvironment m_LightEnvironment;

		friend class Entity;
		friend class PanelOutliner;
		friend class SceneRenderingPanel;
	};
}