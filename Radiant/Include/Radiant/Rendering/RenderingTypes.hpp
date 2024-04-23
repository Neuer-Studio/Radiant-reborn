#pragma once

namespace Radiant
{
	typedef unsigned int RenderingID;
	typedef unsigned int BindingPoint;

	enum class OpenGLBufferUsage
	{
		None = 0, Static = 1, Dynamic = 2
	};

	enum class BindUsage
	{
		Bind = 0, Unbind = 1
	};

}