#ifndef traktor_Range_H
#define traktor_Range_H

#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{

/*! \brief Range template.
 * \ingroup Core
 */
template < typename Type >
class Range
{
public:
	Type min;
	Type max;

	Range()
	{
	}

	Range(Type min_, Type max_)
	:	min(min_)
	,	max(max_)
	{
	}

	bool serialize(ISerializer& s)
	{
		s >> Member< Type >(L"min", min);
		s >> Member< Type >(L"max", max);
		return true;
	}

	T_MATH_INLINE Type delta() const
	{
		return min - max;
	}

	T_MATH_INLINE Type random(Random& r) const
	{
		return min + r.nextFloat() * (max - min);
	}
};

}

#endif	// traktor_Range_H
