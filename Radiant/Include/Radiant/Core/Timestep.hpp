#pragma once

#include <chrono>

namespace Radiant
{
	class Timestep
	{
	public:
		Timestep();
		Timestep(float seconds);

		inline float GetSeconds() const { return m_Time.count(); }
		inline float GetMilliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(m_Time).count(); }

		operator float() const { return m_Time.count(); }

	private:
		std::chrono::duration<float> m_Time;
	};
}