#include <Radiant/Rendering/Environment.hpp>
#include <Radiant/Rendering/SceneRendering.hpp>

namespace Radiant
{
	Environment Environment::Create(const std::string& filepath)
	{
		return SceneRendering::CreateEnvironmentMap(filepath);
	}
}