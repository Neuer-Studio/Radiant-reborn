#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Radiant/Rendering/Mesh.hpp>

#include <entt/entt.hpp>

namespace Radiant
{
	struct Environment;
	struct SceneRendering;
	class Entity;

	class Scene : public Memory::RefCounted
	{
	public:
		Scene(const std::string& sceneName);
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		Entity GetMainCameraEntity();

		void OnUpdate(Timestep ts);
		void SetEnvironment(const Environment& env);
		Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const;

		inline void SetViewportSize(uint32_t width, uint32_t height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const;

		static SceneRendering* GetSceneRendering();
	private:
		std::string m_SceneName;
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;


		friend class Entity;
		friend class PanelOutliner;
	};
}