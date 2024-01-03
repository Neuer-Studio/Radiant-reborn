#pragma once

#include <Radiant/Core/Events/Event.hpp>
#include <Radiant/Core/KeyCodes.hpp>

namespace Radiant
{
	class KeyEvent : public Event {
	public:
		inline KeyCode GetKeyCode() const { return Key; }

		KeyEvent(KeyCode c)
			: Key(c)
		{}
		KeyCode Key;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		virtual const EventType GetEventType() const { return GetStaticType(); }
		static EventType GetStaticType() { return EventType::KeyPressed; }
		inline int GetRepeatCount() const { return RepeatCount; }

		KeyPressedEvent(KeyCode keycode, int repeatCount)
			: KeyEvent(keycode), RepeatCount(repeatCount) {}
		int RepeatCount;
	};
}