#include <Radiant/Rendering/Scene/Components.hpp>

namespace Radiant
{

	Radiant::Component* Component::Create(ComponentType type)
	{
		switch (type)
		{
		case ComponentType::ID:
			return new IDComponent();
		}
		return nullptr; // Unknown component type
	}

}