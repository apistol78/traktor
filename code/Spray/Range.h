#ifndef traktor_spray_Range_H
#define traktor_spray_Range_H

#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Math/Random.h"

namespace traktor
{
	namespace spray
	{

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

	bool serialize(Serializer& s)
	{
		s >> Member< Type >(L"min", min);
		s >> Member< Type >(L"max", max);
		return true;
	}

	inline Type random(Random& r) const
	{
		return min + r.nextFloat() * (max - min);
	}
};

	}
}

#endif	// traktor_spray_Range_H
