#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

/*! Erosion filter.
 * \ingroup Heightfield
 */
class ErosionFilter : public Object
{
	T_RTTI_CLASS;

public:
	explicit ErosionFilter(int32_t iterations);

	void apply(Heightfield* heightfield) const;

private:
	int32_t m_iterations;
};

	}
}
