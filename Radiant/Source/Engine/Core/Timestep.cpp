#include <Radiant/Core/Timestep.hpp>

namespace Radiant
{
	Timestep::Timestep() : m_Time(std::chrono::duration<float>::zero()) {}
	Timestep::Timestep(float seconds) : m_Time(std::chrono::duration<float>(seconds)) {}

}