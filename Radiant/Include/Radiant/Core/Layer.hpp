#pragma once

#include <string>

namespace Radiant
{
	class Layer
	{
	public:
		Layer(const std::string& name = "DebugName");
		virtual ~Layer() {}

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnImGuiRender() = 0;

		inline const std::string& GetName() { return m_Name; }
	private:
		std::string m_Name;
	};
}