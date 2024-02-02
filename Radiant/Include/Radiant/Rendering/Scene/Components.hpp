#pragma once

namespace Radiant
{
	enum class ComponentType
	{
		None = 0,
		ID = 1,
	};

	struct IDComponent;

	struct Component
	{
		virtual const ComponentType GetComponentType() const = 0;
		virtual ~Component() = default;

		static Component* Create(ComponentType type);
	};

	struct IDComponent : public Component
	{
		static const ComponentType GetStaticComponentType() { return ComponentType::ID; }
		virtual const ComponentType GetComponentType() const override { return GetStaticComponentType(); }

		std::string ID;

	};
}