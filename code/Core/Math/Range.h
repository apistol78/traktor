#ifndef traktor_Range_H
#define traktor_Range_H

#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

#undef min
#undef max

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

	Range(const Type& min_, const Type& max_)
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
		return max - min;
	}

	T_MATH_INLINE Type random(Random& r) const
	{
		return min + r.nextFloat() * (max - min);
	}

	template < typename BlendType >
	T_MATH_INLINE Type lerp(const BlendType& b) const
	{
		return min * (BlendType(1) - b) + max * b;
	}

	T_MATH_INLINE Type clamp(const Type& v) const
	{
		if (v < min)
			return min;
		else if (v > max)
			return max;
		else
			return v;
	}

	static T_MATH_INLINE Range< Type > unioon(const Range< Type >& a, const Range< Type >& b)
	{
		return Range< Type >(
			std::min< Type >(a.min, b.min),
			std::max< Type >(a.max, b.max)
		);
	}

	static T_MATH_INLINE Range< Type > intersection(const Range< Type >& a, const Range< Type >& b)
	{
		return Range< Type >(
			std::max< Type >(a.min, b.min),
			std::min< Type >(a.max, b.max)
		);
	}
};

}

#endif	// traktor_Range_H
