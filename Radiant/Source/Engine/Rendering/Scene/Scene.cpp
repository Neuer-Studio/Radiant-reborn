#include <Radiant/Rendering/Scene/Scene.hpp>

namespace Radiant
{

	Environment Scene::CreateScene(const Memory::Shared<SceneRendering> sr)
	{
		return sr->CreateEnvironmentScene("Resources/Textures/HDR/birchwood_4k.hdr");
	}

}