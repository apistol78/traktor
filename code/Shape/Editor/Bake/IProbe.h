#pragma once

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

namespace traktor
{
	namespace shape
	{
	
class IProbe : public Object
{
	T_RTTI_CLASS;

public:
	/*! Sample probe.
	 *
	 * Accumulate from all directions.
	 *
	 * \param direction Array of directions.
	 * \param count Number of directions.
	 * \return Accumulated samples.
	 */
	virtual Color4f sample(const Vector4* directions, uint32_t count) const = 0;
};
	
	}
}
