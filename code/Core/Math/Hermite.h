#ifndef traktor_Hermite_H
#define traktor_Hermite_H

#include "Core/Config.h"
#include "Core/Math/Const.h"

namespace traktor
{

/*! \ingroup Core */
//@{

template <
	typename KeyType,
	typename TimeType,
	typename ValueType
>
struct HermiteDefaultAccessor
{
	static inline TimeType time(const KeyType& key)
	{
		return TimeType(key.T);
	}

	static inline ValueType value(const KeyType& key)
	{
		return ValueType(key.value);
	}

	static inline ValueType combine(
		const ValueType& v0, const TimeType& w0,
		const ValueType& v1, const TimeType& w1,
		const ValueType& v2, const TimeType& w2,
		const ValueType& v3, const TimeType& w3
	)
	{
		return ValueType(v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3);
	}
};

template < typename TimeType >
struct ClampTime
{
	static inline TimeType t(const TimeType& Tat, const TimeType& Tfirst, const TimeType& Tlast, const TimeType& Tend)
	{
		TimeType T = Tat;
		if (T < Tfirst)
			return Tfirst;
		else if (T > Tlast)
			return Tlast;
		else
			return T;
	}

	static inline int index(int i, int nkeys)
	{
		if (i < 0)
			return 0;
		else if (i >= nkeys)
			return nkeys - 1;
		else
			return i;
	}
};

template < typename TimeType >
struct WrapTime
{
	static inline TimeType t(const TimeType& Tat, const TimeType& Tfirst, const TimeType& Tlast, const TimeType& Tend)
	{
		TimeType T = Tat;
		TimeType range = Tend - Tfirst;
		while (T < Tfirst)
			T += range;
		while (T > Tend + Tfirst)
			T -= range;
		return T;
	}

	static inline int index(int i, int nkeys)
	{
		while (i < 0)
			i += nkeys;
		while (i >= nkeys)
			i -= nkeys;
		return i;
	}
};

template <
	typename KeyType,
	typename TimeType,
	typename ValueType,
	typename Accessor = HermiteDefaultAccessor< KeyType, TimeType, ValueType >,
	typename TimeControl = ClampTime< TimeType >
>
struct Hermite
{
	static ValueType evaluate(const KeyType* keys, size_t nkeys, const TimeType& Tat, const TimeType& Tend = TimeType(-1.0f), const TimeType& stiffness = TimeType(0.5f))
	{
		T_ASSERT (nkeys >= 2);

		TimeType Tfirst(Accessor::time(keys[0]));
		TimeType Tlast(Accessor::time(keys[nkeys - 1]));
		TimeType Tcurr = TimeControl::t(Tat, Tfirst, Tlast, Tend > TimeType(0.0f) ? Tend : Tlast);

		int index = 0;
		while (index < int(nkeys - 1) && Tcurr >= Accessor::time(keys[index + 1]))
			++index;

		int index_n1 = TimeControl::index(index - 1, int(nkeys));
		int index_1 = TimeControl::index(index + 1, int(nkeys));
		int index_2 = TimeControl::index(index + 2, int(nkeys));
		
		const KeyType& cp0 = keys[index];
		const KeyType& cp1 = keys[index_1];

		ValueType v0 = Accessor::value(cp0);
		ValueType v1 = Accessor::value(cp1);

		TimeType t0(Accessor::time(cp0));
		TimeType t1(Accessor::time(cp1));

		if (t0 >= t1)
			t1 = Tend;

		const KeyType& cpp = keys[index_n1];
		const KeyType& cpn = keys[index_2];

		ValueType vp = Accessor::value(cpp);
		ValueType vn = Accessor::value(cpn);

		TimeType t = (Tcurr - t0) / (t1 - t0);
		TimeType t2 = t * t;
		TimeType t3 = t2 * t;

		TimeType h2 = TimeType(3.0f) * t2 - t3 - t3;
		TimeType h1 = TimeType(1.0f) - h2;
		TimeType h4 = t3 - t2;
		TimeType h3 = h4 - t2 + t;

		h3 *= stiffness;
		h4 *= stiffness;

		return Accessor::combine(
			v0, h1 - h4,
			v1, h2 + h3,
			vp, -h3,
			vn, h4
		);
	}
};

//@}

}

#endif	// traktor_Hermite_H
