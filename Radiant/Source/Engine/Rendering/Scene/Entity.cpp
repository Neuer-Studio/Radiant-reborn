#include <Radiant/Rendering/Scene/Entity.hpp>

namespace Radiant
{

	Entity::Entity(const std::string& name)
		: m_UUID()
	{
		m_Components[ComponentType::ID] = Component::Create(ComponentType::ID);
		IDComponent* c = (IDComponent*)m_Components[ComponentType::ID];
		c->ID = name;
	}

	Entity::~Entity()
	{
		for (const auto c : m_Components)
		{
			delete c.second;
		}
	}

}