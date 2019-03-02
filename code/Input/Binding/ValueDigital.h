#pragma once

#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace input
	{

/*! \brief
 * \ingroup Input
 */
bool T_FORCE_INLINE asBoolean(float inputValue)
{
	return inputValue >= 0.5f ? true : false;
}

/*! \brief
 * \ingroup Input
 */
float T_FORCE_INLINE asFloat(bool inputValue)
{
	return inputValue ? 1.0f : 0.0f;
}

	}
}

