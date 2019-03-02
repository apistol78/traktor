#pragma once

#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

// Verbose logging.
#if defined(_DEBUG)
#	define T_IF_VERBOSE(x) { x }
#else
#	define T_IF_VERBOSE(x)
#endif

	}
}

