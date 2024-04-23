#include <Radiant/Rendering/Environment.hpp>
#include <Radiant/Scene/SceneRendering.hpp>

namespace Radiant
{
	Environment Environment::Create(const std::string& filepath)
	{
		return SceneRendering::CreateEnvironmentMap(filepath);
	}
}