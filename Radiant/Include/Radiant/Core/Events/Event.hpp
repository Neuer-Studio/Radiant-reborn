#pragma once

namespace Radiant
{
	enum class EventType
	{
		None = 0, 
		WindowClose, WindowResize, // Window
		KeyPressed // Keys
	};

	class Event 
	{
	public:
		virtual const EventType GetEventType() const = 0;

		bool m_Handled = false;
	};

	class EventManager final // NOTE: Should be static ? 
	{
	public:
		template <typename T>
		using EventFN = std::function<bool(const T&)>;
		
		EventManager(Event& e)
			: m_Event(e) {}

		template <typename T>
		bool Notify(EventFN<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

		Event& m_Event;

	};

}